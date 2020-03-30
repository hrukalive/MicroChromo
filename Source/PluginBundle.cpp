/*
  ==============================================================================

    PluginBundle.cpp
    Created: 8 Mar 2020 9:54:48am
    Author:  bowen

  ==============================================================================
*/

#include "PluginBundle.h"
#include "PluginProcessor.h"

PluginBundle::PluginBundle(MicroChromoAudioProcessor& p, int maxInstances, OwnedArray<ParameterLinker>& linker, PluginDescription emptyPlugin, PluginDescription defaultPlugin)
    : _maxInstances(maxInstances), _numInstances(1), processor(p), formatManager(p.getAudioPluginFormatManager()), parameterLinker(linker), _emptyPlugin(emptyPlugin), _defaultPlugin(defaultPlugin)
{
    for (auto i = 0; i < maxInstances; i++)
        collectors.set(i, new MidiMessageCollector());
    ccLearn = std::make_unique<ParameterCcLearn>(this);
}

PluginBundle::~PluginBundle()
{
    ccLearn = nullptr;
    activePluginWindows.clear();
    instances.clear();
    instanceTemps.clear();
    collectors.clear();
}

void PluginBundle::preLoadPlugin(int numInstances)
{
    jassert(numInstances <= _maxInstances);
    if (_isLoading.load())
    {
        AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, "Please Wait", "Wait for current loading to finish", "OK");
        return;
    }
    _isLoading = true;
    _isLoaded = false;
    _finishedLoading = false;
    _isError = false;
    instanceStartedTemp = 0;

    sendSynchronousChangeMessage();
    processor.startLoadingPlugin();
    instanceTemps.clearQuick(false);
}

void PluginBundle::loadPlugin(const PluginDescription desc, int numInstances, std::function<void(PluginBundle&)> callback)
{
    jassert(desc.name.isNotEmpty());
    preLoadPlugin(numInstances);
    for (auto i = 0; i < numInstances; i++)
    {
        formatManager.createPluginInstanceAsync(desc,
            processor.getSampleRate(),
            processor.getBlockSize(),
            [this, i, desc, numInstances, callback](std::unique_ptr<AudioPluginInstance> instance, const String& error)
            {
                addPluginCallback(std::move(instance), error, i);
                checkPluginLoaded(desc, numInstances, callback);
            });
    }
}

void PluginBundle::loadPluginSync(const PluginDescription desc, int numInstances)
{
    jassert(desc.name.isNotEmpty());
    preLoadPlugin(numInstances);
    for (auto i = 0; i < numInstances; i++)
    {
        String error;
        auto instance = formatManager.createPluginInstance(desc, processor.getSampleRate(), processor.getBlockSize(), error);
        addPluginCallback(std::move(instance), error, i);
    }
    checkPluginLoaded(desc, numInstances);
}

//PluginInstance* PluginBundle::getInstanceAt(size_t index)
//{
//    if (index >= instanceStarted.load())
//        return nullptr;
//    return instances[index];
//}

MidiMessageCollector* PluginBundle::getCollectorAt(int index)
{
    if (index >= instanceStarted.load())
        return nullptr;
    return collectors[index];
}

void PluginBundle::addPluginCallback(std::unique_ptr<AudioPluginInstance> instance, const String& error, int index)
{
    if (instance == nullptr)
    {
        _isError = true;
        errMsg = error;
    }
    else
    {
        instance->enableAllBuses();
        instanceTemps.set(index, new PluginInstance(uid++, std::move(instance)), true);
        instanceStartedTemp++;
    }
}

void PluginBundle::checkPluginLoaded(const PluginDescription desc, int numInstances, std::function<void(PluginBundle&)> callback)
{
    if (instanceStartedTemp.load() == numInstances)
    {
        _isLoading = false;
        if (_isError.load())
        {
            AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon,
                "Loading Failed",
                TRANS("Couldn't create plugin. ") + errMsg,
                "OK");
            _isLoaded = false;
            if (!isInit)
                _isLoaded = true;
        }
        else
        {
            if (!isInit)
            {
                for (auto* p : instances[0]->processor->getParameters())
                    p->removeListener(this);
            }
            else
                isInit = false;
            instanceStarted = instanceStartedTemp.load();
            _numInstances = numInstances;

            hasLearned = false;
            resetCcLearn();
            resetParameterLink();
            linkParameterIndices.clear();

            instances.clear();
            for (auto i = 0; i < _numInstances; i++)
                instances.set(i, instanceTemps[i], true);
            instanceTemps.clearQuick(false);

            for (auto* p : instances[0]->processor->getParameters())
                p->addListener(this);

            if (callback)
                callback(*this);

            if (!currentDesc.isDuplicateOf(desc))
                currentDesc = desc;

            _isLoaded = true;
        }
        _finishedLoading = true;
        sendChangeMessage();
        processor.finishLoadingPlugin();
    }
}

void PluginBundle::adjustInstanceNumber(int newNumInstances, std::function<void(void)> callback)
{
    MemoryBlock data;
    getStateInformation(data);
    loadPlugin(currentDesc, newNumInstances, [data, callback](PluginBundle& bundle)
        {
            bundle.setStateInformation(data.getData(), data.getSize());
            if (callback)
                callback();
        });
}

void PluginBundle::openParameterLinkEditor()
{
    DialogWindow::LaunchOptions dialogOption;

    dialogOption.dialogTitle = "Choose Parameters to Expose";
    dialogOption.dialogBackgroundColour = LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId);
    dialogOption.escapeKeyTriggersCloseButton = false;
    dialogOption.useNativeTitleBar = false;
    dialogOption.resizable = true;
    dialogOption.content.setOwned(new ParameterLinkEditor(*this));
    dialogOption.launchAsync();
}

void PluginBundle::resetParameterLink()
{
    for (auto& ptr : parameterLinker)
        ptr->resetLink();
}

void PluginBundle::linkParameters()
{
    int i = 0;
    auto& parameters = instances[0]->processor->getParameters();
    std::sort(linkParameterIndices.begin(), linkParameterIndices.end());
    auto learnedCc = getLearnedCc();
    std::remove_if(linkParameterIndices.begin(), linkParameterIndices.end(), [&parameters, learnedCc](int v) { return v >= parameters.size() || v == learnedCc; });
    for (auto index : linkParameterIndices)
    {
        if (i >= processor.getParameterSlotNumber())
            break;
        auto* p = parameters[index];
        if (p->isAutomatable() && !p->isMetaParameter())
        {
            parameterLinker[i]->linkParameter(index, p);
            i++;
        }
    }
    processor.updateHostDisplay();
}

void PluginBundle::linkEditorExit(Array<int> selected)
{
    linkParameterIndices = selected;
    resetParameterLink();
    linkParameters();
}

void PluginBundle::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    _sampleRate = sampleRate;
    _samplesPerBlock = samplesPerBlock;
    for (int i = 0; i < instanceStarted.load(); i++)
    {
        instances[i]->prepare(sampleRate, samplesPerBlock);
        collectors[i]->reset(sampleRate);
    }
}

void PluginBundle::processBlock(OwnedArray<AudioBuffer<float>>& bufferArray, MidiBuffer& midiMessages)
{
    if (isLoaded())
    {
        for (auto i = 0; i < instanceStarted.load(); i++)
        {
            MidiBuffer midiBuffer(midiMessages);
            collectors[i]->removeNextBlockOfMessages(midiBuffer, _samplesPerBlock.load());
            MidiMessage midi_message;
            int sample_offset;

            for (MidiBuffer::Iterator it(midiBuffer); it.getNextEvent(midi_message, sample_offset);) {
                if (midi_message.isController()) {
                    ccLearn->processCc(i, midi_message.getControllerNumber(), midi_message.getControllerValue(), sample_offset);
                }
            }
            instances[i]->processor->processBlock(*bufferArray[i], midiBuffer);
        }
    }
}

void PluginBundle::releaseResources()
{
    for (int i = 0; i < instanceStarted.load(); i++)
        instances[i]->unprepare();
}

void PluginBundle::propagateState()
{
    MemoryBlock block;
    instances[0]->processor->getStateInformation(block);
    for (auto i = 1; i < instanceStarted.load(); i++)
        instances[i]->processor->setStateInformation(block.getData(), block.getSize());
}

const Array<AudioProcessorParameter*>& PluginBundle::getParameters(int index)
{
    return instances[index]->processor->getParameters();
}

void PluginBundle::setParameterValue(int parameterIndex, float newValue)
{
    for (auto i = instanceStarted.load() - 1; i >= 0; i--)
        instances[i]->processor->getParameters()[parameterIndex]->setValue(newValue);
}

void PluginBundle::setParameterGesture(int parameterIndex, bool gestureIsStarting)
{
    for (auto i = instanceStarted.load() - 1; i >= 0; i--)
    {
        if (gestureIsStarting)
            instances[i]->processor->getParameters()[parameterIndex]->beginChangeGesture();
        else
            instances[i]->processor->getParameters()[parameterIndex]->endChangeGesture();
    }
}

void PluginBundle::parameterValueChanged(int parameterIndex, float newValue)
{
    if (isLearning)
    {
        learnedCc = parameterIndex;
        if (newValue > learnedCcMax)
            learnedCcMax = newValue;
        else if (newValue < learnedCcMin)
            learnedCcMin = newValue;
    }
    for (auto i = 1; i < instanceStarted.load(); i++)
        instances[i]->processor->getParameters()[parameterIndex]->setValue(newValue);
}

void PluginBundle::parameterGestureChanged(int parameterIndex, bool gestureIsStarting)
{
    for (auto i = 1; i < instanceStarted.load(); i++)
    {
        if (gestureIsStarting)
            instances[i]->processor->getParameters()[parameterIndex]->beginChangeGesture();
        else
            instances[i]->processor->getParameters()[parameterIndex]->endChangeGesture();
    }
}

void PluginBundle::resetCcLearn()
{
    ccLearn->reset();
}

void PluginBundle::startCcLearn()
{
    ccLearn->startLearning();
}

void PluginBundle::setCcLearn(int ccNum, int index, float min, float max)
{
    ccLearn->setCcLearn(ccNum, index, min, max);
}

void PluginBundle::getStateInformation(MemoryBlock& destData)
{
    std::unique_ptr<XmlElement> xml = std::make_unique<XmlElement>("bundle");

    MemoryBlock ccData;
    ccLearn->getStateInformation(ccData);
    xml->setAttribute("ccData", ccData.toBase64Encoding());
    
    MemoryBlock linkedData;
    MemoryOutputStream stream(linkedData, false);
    for (int i : linkParameterIndices)
        stream.writeInt(i);
    stream.flush();
    xml->setAttribute("linkParameterIndices", linkedData.toBase64Encoding());

    MemoryBlock data;
    instances[0]->processor->getStateInformation(data);
    xml->setAttribute("data", data.toBase64Encoding());
    AudioProcessor::copyXmlToBinary(*xml, destData);
}

void PluginBundle::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xml(AudioProcessor::getXmlFromBinary(data, sizeInBytes));
    if (xml.get() != nullptr)
    {
        if (xml->hasTagName("bundle"))
        {
            if (xml->hasAttribute("ccData"))
            {
                MemoryBlock ccData;
                ccData.fromBase64Encoding(xml->getStringAttribute("ccData"));
                ccLearn->setStateInformation(ccData.getData(), ccData.getSize());
            }
            if (xml->hasAttribute("data"))
            {
                MemoryBlock proc_data;
                proc_data.fromBase64Encoding(xml->getStringAttribute("data"));
                for (auto i = 0; i < instanceStarted.load(); i++)
                    instances[i]->processor->setStateInformation(proc_data.getData(), proc_data.getSize());
            }
            if (xml->hasAttribute("linkParameterIndices"))
            {
                MemoryBlock linkedData;
                linkedData.fromBase64Encoding(xml->getStringAttribute("linkParameterIndices"));
                linkParameterIndices.clear();
                MemoryInputStream stream(linkedData, false);
                while (!stream.isExhausted())
                    linkParameterIndices.add(stream.readInt());
                resetParameterLink();
                linkParameters();
            }
        }
    }
}

void PluginBundle::closeAllWindows()
{
    activePluginWindows.clear();
}

void PluginBundle::showRepresentativeWindow()
{
    showWindow(PluginWindow::Type::normal, 1);
}

void PluginBundle::showTwoWindows()
{
    showWindow(PluginWindow::Type::normal, jmin(instanceStarted.load(), 2));
}

void PluginBundle::showAllWindows()
{
    showWindow(PluginWindow::Type::normal, instanceStarted.load());
}

void PluginBundle::showWindow(PluginWindow::Type type, int num)
{
    for (auto i = num - 1; i >= 0; i--)
    {
        if (auto node = instances[i])
        {
            for (auto* w : activePluginWindows)
            {
                if (w->node == node && w->type == type)
                {
                    w->toFront(true);
                    return;
                }
            }
            if (node->processor != nullptr)
            {
                activePluginWindows.add(new PluginWindow(node, type, activePluginWindows))->toFront(true);
            }
        }
    }
}

std::unique_ptr<PopupMenu> PluginBundle::getPopupMenu(KnownPluginList::SortMethod pluginSortMethod, KnownPluginList& knownPluginList)
{
    std::unique_ptr<PopupMenu> floatMenu = std::make_unique<PopupMenu>();

    floatMenu->addItem(SLOT_MENU_SHOW_MAIN_GUI, "Show main plugin GUI");
    floatMenu->addItem(SLOT_MENU_SHOW_TWO_GUI, "Show two plugin GUI", processor.getNumInstances() > 1);
    floatMenu->addItem(SLOT_MENU_SHOW_ALL_GUI, "Show all plugin GUI", processor.getNumInstances() > 2);
    floatMenu->addItem(SLOT_MENU_CLOSE_ALL_GUI, "Close all window");
    floatMenu->addItem(SLOT_MENU_SHOW_PROGRAMS, "Show programs");
    floatMenu->addItem(SLOT_MENU_SHOW_PARAMETERS, "Show parameters");
    floatMenu->addItem(SLOT_MENU_SHOW_DEBUG_LOG, "Show debug log");
    floatMenu->addSeparator();
    floatMenu->addItem(SLOT_MENU_PROPAGATE_STATE, "Propagate state to duplicates");
    floatMenu->addItem(SLOT_MENU_EXPOSE_PARAMETER, "Expose parameters");
    floatMenu->addItem(SLOT_MENU_START_CC, "Start CC Learn", !ccLearn->isLearning());
    floatMenu->addItem(SLOT_MENU_SHOW_CC, "Show CC Status");
    floatMenu->addItem(SLOT_MENU_CLEAR_CC, "Clear CC Learn");
    floatMenu->addSeparator();
    floatMenu->addItem(SLOT_MENU_LOAD_EMPTY_PLUGIN, _emptyPlugin.name, true, currentDesc.isDuplicateOf(_emptyPlugin));
    floatMenu->addItem(SLOT_MENU_LOAD_DEFAULT_PLUGIN, _defaultPlugin.name, true, currentDesc.isDuplicateOf(_defaultPlugin));
    floatMenu->addSeparator();
    KnownPluginList::addToMenu(*floatMenu, knownPluginList.getTypes(), pluginSortMethod, currentDesc.createIdentifierString());

    return floatMenu;
}
