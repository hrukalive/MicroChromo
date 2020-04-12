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
    ccLearn = std::make_unique<ParameterCcLearn>(this);
}

PluginBundle::~PluginBundle()
{
    ccLearn = nullptr;
    activePluginWindows.clear();
    instances.clear();
    instanceTemps.clear();
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

            ccLearn->reset();
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

            if (!currentDesc.isDuplicateOf(desc))
                currentDesc = desc;

            if (callback)
                callback(*this);

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
    auto xml = *createXml("root");
    loadPlugin(currentDesc, newNumInstances, [xml, callback](PluginBundle& bundle)
        {
            bundle.loadFromXml(&xml, "root");
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
        instances[i]->prepare(sampleRate, samplesPerBlock);
}

void PluginBundle::processBlock(OwnedArray<AudioBuffer<float>>& bufferArray, OwnedArray<MidiBuffer>& midiMessagesArray)
{
    if (isLoaded())
    {
        for (auto i = 0; i < instanceStarted.load(); i++)
        {
            auto& midiBuffer = *midiMessagesArray[i];
            MidiMessage midi_message;
            int sample_offset;

            for (MidiBuffer::Iterator it(midiBuffer); it.getNextEvent(midi_message, sample_offset);) {
                if (midi_message.isController()) {
                    ccLearn->processCc(i, midi_message.getControllerNumber(), midi_message.getControllerValue(), sample_offset);
                }
            }
            instances[i]->processor->processBlock(*bufferArray[i], midiBuffer);
            midiBuffer.clear();
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

std::unique_ptr<XmlElement> PluginBundle::createXml(String rootTag)
{
    std::unique_ptr<XmlElement> xml = std::make_unique<XmlElement>(rootTag);

    xml->addChildElement(new XmlElement(*getDescription().createXml()));
    xml->addChildElement(new XmlElement(*ccLearn->createXml()));

    XmlElement* linkParameterIndicesRoot = new XmlElement("linkParameterIndices");
    for (int val : linkParameterIndices)
    {
        auto* element = linkParameterIndicesRoot->createNewChildElement("item");
        element->setAttribute("value", val);
    }
    xml->addChildElement(linkParameterIndicesRoot);

    XmlElement* linkParameterHistoryRoot = new XmlElement("linkParameterHistory");
    for (auto& x : linkParameterIndicesHistory)
    {
        auto* element = linkParameterHistoryRoot->createNewChildElement("record");
        element->setAttribute("key", x.first);
        element->setAttribute("value", x.second);
    }
    xml->addChildElement(linkParameterHistoryRoot);

    MemoryBlock data;
    instances[0]->processor->getStateInformation(data);
    xml->setAttribute("processorData", data.toBase64Encoding());

    return xml;
}

void PluginBundle::loadFromXml(const XmlElement* xml, String rootTag)
{
    if (xml != nullptr && xml->getTagName() == rootTag)
    {
        if (xml->hasAttribute("processorData"))
        {
            MemoryBlock proc_data;
            proc_data.fromBase64Encoding(xml->getStringAttribute("processorData"));
            for (auto i = 0; i < instanceStarted.load(); i++)
                instances[i]->processor->setStateInformation(proc_data.getData(), proc_data.getSize());
        }

        ccLearn->loadFromXml(xml->getChildByName("ccLearnModule"));

        if (auto* linkParameterIndicesRoot = xml->getChildByName("linkParameterIndices"))
        {
            if (auto* linkParameterHistoryRoot = xml->getChildByName("linkParameterHistory"))
            {
                linkParameterIndicesHistory.clear();
                forEachXmlChildElementWithTagName(*linkParameterHistoryRoot, record, "record")
                {
                    linkParameterIndicesHistory[record->getIntAttribute("key", -1)] = record->getIntAttribute("value", -1);
                }
            }
            if (linkParameterIndicesRoot->getNumChildElements() > 0)
            {
                linkParameterIndices.clear();
                forEachXmlChildElementWithTagName(*linkParameterIndicesRoot, item, "item")
                {
                    linkParameterIndices.add(item->getIntAttribute("value", -1));
                }
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
    floatMenu->addItem(SLOT_MENU_START_CC, "Start CC Learn", canLearnCc && !ccLearn->isLearning());
    floatMenu->addItem(SLOT_MENU_SHOW_CC, "Show CC Status", ccLearn->hasLearned());
    floatMenu->addItem(SLOT_MENU_CLEAR_CC, "Clear CC Learn", ccLearn->hasLearned());
    if (this == processor.getSynthBundlePtr().get() && isKontakt())
    {
        floatMenu->addSeparator();
        floatMenu->addItem(SLOT_MENU_USE_KONTAKT, "Kontakt Specific", canChooseKontakt, processor.getPitchShiftModulationSource() == USE_KONTAKT);
        floatMenu->addItem(SLOT_MENU_COPY_KONTAKT_SCRIPT, "Copy Kontakt Script", canChooseKontakt);

        PopupMenu subMenu;
        for (int i = 0; i < 128 - 12 + 1; i++)
            subMenu.addItem(CC_VALUE_BASE + i, "CC " + String(i), true, ccBase == i);
        floatMenu->addSubMenu("Kontakt CC Number", subMenu, processor.getPitchShiftModulationSource() == USE_KONTAKT);
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
