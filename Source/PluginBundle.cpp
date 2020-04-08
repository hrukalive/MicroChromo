/*
  ==============================================================================

    PluginBundle.cpp
    Created: 8 Mar 2020 9:54:48am
    Author:  bowen

  ==============================================================================
*/

#include "PluginBundle.h"
#include "PluginProcessor.h"

PluginBundle::PluginBundle(MicroChromoAudioProcessor& p, 
    int maxInstances, 
    OwnedArray<ParameterLinker>& linker, 
    PluginDescription emptyPlugin, 
    PluginDescription defaultPlugin)
    : _maxInstances(maxInstances), 
    _numInstances(1), 
    processor(p), 
    formatManager(p.getAudioPluginFormatManager()), 
    parameterLinker(linker), 
    _emptyPlugin(emptyPlugin), 
    _defaultPlugin(defaultPlugin)
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
    _isNewlyLoaded = false;
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
    if (desc.isDuplicateOf(currentDesc) && _numInstances == numInstances)
        return;
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
    if (desc.isDuplicateOf(currentDesc) && _numInstances == numInstances)
        return;
    preLoadPlugin(numInstances);
    for (auto i = 0; i < numInstances; i++)
    {
        String error;
        auto instance = formatManager.createPluginInstance(desc, processor.getSampleRate(), processor.getBlockSize(), error);
        addPluginCallback(std::move(instance), error, i);
    }
    checkPluginLoaded(desc, numInstances);
}

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

            resetCcLearn();
            resetParameterLink();
            linkParameterIndices.clear();
            linkParameterIndicesHistory.clear();
            linkParameterIndicesSet.clear();

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

            _isNewlyLoaded = true;
        }
        _isLoaded = _isLoaded || _isNewlyLoaded;
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

void PluginBundle::clearMidiCollectorBuffer()
{
    if (isLoaded())
        for (int i = 0; i < instanceStarted.load(); i++)
            collectors[i]->reset(_sampleRate);
}

void PluginBundle::addMessageToAllQueue(MidiMessage& msg)
{
    if (isLoaded())
        for (int i = 0; i < instanceStarted.load(); i++)
            collectors[i]->addMessageToQueue(msg);
}

void PluginBundle::sendAllNotesOff()
{
    if (isLoaded())
    {
        for (int i = 0; i < instanceStarted.load(); i++)
        {
            collectors[i]->reset(_sampleRate);
            for (auto j = 1; j <= 16; j++)
            {
                MidiMessage msg1(MidiMessage::allNotesOff(j));
                MidiMessage msg2(MidiMessage::allSoundOff(j));
                MidiMessage msg3(MidiMessage::allControllersOff(j));
                msg1.setTimeStamp(0.001);
                msg2.setTimeStamp(0.001);
                msg3.setTimeStamp(0.001);
                collectors[i]->addMessageToQueue(msg1);
                collectors[i]->addMessageToQueue(msg2);
                collectors[i]->addMessageToQueue(msg3);
            }
        }
    }
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

struct PairComparator
{
    int compareElements(std::pair<int, int> first, std::pair<int, int> second)
    {
        if (first.second < second.second)
            return -1;
        if (first.second == second.second)
            return 0;
        if (first.second > second.second)
            return 1;
    }
};

void PluginBundle::linkParameters()
{
    auto& parameters = instances[0]->processor->getParameters();
    linkParameterIndices.sort();
    auto learnedCc = getLearnedCcParameterIndex();
    linkParameterIndices.removeIf([&parameters, learnedCc](int v) { return v >= parameters.size() || v == learnedCc; });
    linkParameterIndicesSet.clear();
    for (auto index : linkParameterIndices)
        linkParameterIndicesSet.insert(index);

    Array<int> emptyIndices;
    for (int i = 0; i < processor.getParameterSlotNumber(); i++)
        emptyIndices.add(i);
    for (auto& x : linkParameterIndicesHistory)
        emptyIndices.removeAllInstancesOf(x.second);

    Array<std::pair<int, int>> overrideIndices;
    for (auto x : linkParameterIndicesHistory)
        if (!linkParameterIndicesSet.contains(x.first) && x.second > -1 && x.second < processor.getParameterSlotNumber())
            overrideIndices.add(x);
    overrideIndices.sort(PairComparator(), true);

    int linkedCount = 0, j = 0;
    for (auto index : linkParameterIndices)
    {
        if (linkedCount >= processor.getParameterSlotNumber())
            break;
        auto* p = parameters[index];
        if (p->isAutomatable() && !p->isMetaParameter())
        {
            if (linkParameterIndicesHistory.find(index) != linkParameterIndicesHistory.end())
            {
                auto i = linkParameterIndicesHistory[index];
                if (i > -1 && i < processor.getParameterSlotNumber())
                {
                    parameterLinker[i]->linkParameter(index, p);
                    linkedCount++;
                    continue;
                }
                else
                {
                    linkParameterIndicesHistory.erase(index);
                }
            }

            if (emptyIndices.size() > 0)
            {
                parameterLinker[emptyIndices[0]]->linkParameter(index, p);
                linkedCount++;
                linkParameterIndicesHistory[index] = emptyIndices[0];
                emptyIndices.remove(&emptyIndices.getReference(0));
            }
            else if (overrideIndices.size() > 0)
            {
                auto pair = overrideIndices[0];
                linkParameterIndicesHistory.erase(pair.first);
                parameterLinker[pair.second]->linkParameter(index, p);
                linkedCount++;
                linkParameterIndicesHistory[index] = pair.second;
                overrideIndices.remove(&overrideIndices.getReference(0));
            }
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

void PluginBundle::processBlock(OwnedArray<AudioBuffer<float>>& bufferArray, MidiBuffer& /*midiMessages*/)
{
    if (isLoaded())
    {
        for (auto i = 0; i < instanceStarted.load(); i++)
        {
            MidiBuffer midiBuffer;
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

bool PluginBundle::hasCcLearned()
{
    return ccLearn->hasLearned();
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

    MemoryBlock linkedKeyData, linkedValueData;
    MemoryOutputStream keyStream(linkedKeyData, false), valueStream(linkedValueData, false);
    for (auto& x : linkParameterIndicesHistory)
    {
        keyStream.writeInt(x.first);
        valueStream.writeInt(x.second);
    }
    keyStream.flush();
    valueStream.flush();
    xml->setAttribute("linkParameterHistoryKey", linkedKeyData.toBase64Encoding());
    xml->setAttribute("linkParameterHistoryValue", linkedValueData.toBase64Encoding());

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
                if (xml->hasAttribute("linkParameterHistoryKey") && xml->hasAttribute("linkParameterHistoryValue"))
                {
                    MemoryBlock linkedKeyData, linkedValueData;
                    linkedKeyData.fromBase64Encoding(xml->getStringAttribute("linkParameterHistoryKey"));
                    linkedValueData.fromBase64Encoding(xml->getStringAttribute("linkParameterHistoryValue"));
                    MemoryInputStream keyStream(linkedKeyData, false), valueStream(linkedValueData, false);
                    Array<int> keyArr, valArr;
                    while (!keyStream.isExhausted())
                        keyArr.add(keyStream.readInt());
                    while (!valueStream.isExhausted())
                        valArr.add(valueStream.readInt());
                    if (keyArr.size() == valArr.size())
                    {
                        linkParameterIndicesHistory.clear();
                        for (int i = 0; i < keyArr.size(); i++)
                            linkParameterIndicesHistory[keyArr[i]] = valArr[i];
                    }
                }

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

void PluginBundle::showWindow(int num, PluginWindow::Type type)
{
    num = jmin(instanceStarted.load(), num);
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
                auto w = new PluginWindow(node, type, activePluginWindows, i == 0);
                activePluginWindows.add(w)->toFront(true);
            }
        }
    }
}

void PluginBundle::bringToFront()
{
    for (auto* w : activePluginWindows)
        w->toFront(false);
}

std::unique_ptr<PopupMenu> PluginBundle::getMainPopupMenu()
{
    std::unique_ptr<PopupMenu> floatMenu = std::make_unique<PopupMenu>();

    auto canLearnCc = processor.canLearnCc(this);
    auto canChooseKontakt = processor.canChooseKontakt();
    auto ccBase = processor.getCcBase();

    floatMenu->addItem(SLOT_MENU_SHOW_MAIN_GUI, "Show representative plugin GUI");
    floatMenu->addItem(SLOT_MENU_SHOW_TWO_GUI, "Show two plugin GUI", processor.getNumInstances() > 1);
    floatMenu->addItem(SLOT_MENU_SHOW_ALL_GUI, "Show all plugin GUI", processor.getNumInstances() > 2);
    floatMenu->addItem(SLOT_MENU_CLOSE_ALL_GUI, "Close all window");
    floatMenu->addSeparator();
    floatMenu->addItem(SLOT_MENU_SHOW_PROGRAMS, "Show programs");
    floatMenu->addItem(SLOT_MENU_SHOW_PARAMETERS, "Show parameters");
    floatMenu->addItem(SLOT_MENU_SHOW_DEBUG_LOG, "Show debug log");
    floatMenu->addSeparator();
    floatMenu->addItem(SLOT_MENU_PROPAGATE_STATE, "Propagate state to duplicates");
    floatMenu->addItem(SLOT_MENU_EXPOSE_PARAMETER, "Expose parameters");
    floatMenu->addSeparator();
    floatMenu->addItem(SLOT_MENU_START_CC, "Start CC Learn", !ccLearn->isLearning() && canLearnCc);
    floatMenu->addItem(SLOT_MENU_SHOW_CC, "Show CC Status", canLearnCc);
    floatMenu->addItem(SLOT_MENU_CLEAR_CC, "Clear CC Learn", ccLearn->hasLearned());
    //if (isKontakt())
    {
        floatMenu->addSeparator();
        floatMenu->addItem(SLOT_MENU_USE_KONTAKT, "Kontakt Specific", canChooseKontakt, processor.getPitchShiftModulationSource() == USE_KONTAKT);
        //floatMenu->addItem(SLOT_MENU_KONTAKT_CC, "Starting CC Number", canChooseKontakt, processor.getPitchShiftModulationSource() == USE_KONTAKT);

        PopupMenu subMenu;
        for (int i = 0; i < 128 - 12 + 1; i++)
            subMenu.addItem(CC_VALUE_BASE + i, "CC " + String(i), true, ccBase == i);
        floatMenu->addSubMenu("Kontakt CC Number", subMenu, canChooseKontakt);
    }

    return floatMenu;
}

std::unique_ptr<PopupMenu> PluginBundle::getPluginPopupMenu(KnownPluginList::SortMethod pluginSortMethod, KnownPluginList& knownPluginList)
{
    std::unique_ptr<PopupMenu> floatMenu = std::make_unique<PopupMenu>();
    floatMenu->addItem(SLOT_MENU_LOAD_EMPTY_PLUGIN, _emptyPlugin.name, true, currentDesc.isDuplicateOf(_emptyPlugin));
    floatMenu->addItem(SLOT_MENU_LOAD_DEFAULT_PLUGIN, _defaultPlugin.name, true, currentDesc.isDuplicateOf(_defaultPlugin));
    floatMenu->addSeparator();
    KnownPluginList::addToMenu(*floatMenu, knownPluginList.getTypes(), pluginSortMethod, currentDesc.createIdentifierString());
    return floatMenu;
}

bool PluginBundle::isKontakt()
{
    if (currentDesc.name.toLowerCase().indexOf("kontakt") > -1)
        return true;
    return false;
}
