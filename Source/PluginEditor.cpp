/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
class MicroChromoAudioProcessorEditor::PluginListWindow : public DocumentWindow
{
public:
    PluginListWindow(MicroChromoAudioProcessorEditor &mw, AudioPluginFormatManager& pluginFormatManager)
        : DocumentWindow("Available Plugins",
            LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId),
            DocumentWindow::minimiseButton | DocumentWindow::closeButton),
        owner(mw)
    {
        auto deadMansPedalFile = owner.appProperties.getUserSettings()->getFile().getSiblingFile("RecentlyCrashedPluginsList");

        setContentOwned(new PluginListComponent(pluginFormatManager, owner.knownPluginList, deadMansPedalFile, owner.appProperties.getUserSettings(), true), true);

        setResizable(true, false);
        setResizeLimits(300, 400, 800, 1500);
        setTopLeftPosition(60, 60);

        setVisible(true);
    }

    ~PluginListWindow()
    {
        clearContentComponent();
    }

    void closeButtonPressed()
    {
        owner.pluginListWindow = nullptr;
    }

private:
    MicroChromoAudioProcessorEditor& owner;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginListWindow)
};

//==============================================================================
MicroChromoAudioProcessorEditor::MicroChromoAudioProcessorEditor (MicroChromoAudioProcessor& p)
    : AudioProcessorEditor (&p), 
    processor (p), 
    appProperties(p.getApplicationProperties()),
    knownPluginList(p.getKnownPluginList()),
    formatManager(p.getAudioPluginFormatManager()),
    synthBundle(p.getSynthBundlePtr()),
    psBundle(p.getPSBundlePtr())
{
    menuBar.reset(new MenuBarComponent(this));
    menuBar->setVisible(true);

    synthBtn.reset(new TextButton("Synth"));
    psBtn.reset(new TextButton("PitchShift"));
    noteButton.reset(new TextButton("Note"));
    ccLearnBtn.reset(new ToggleButton("CC Learn"));
    dragBtn.reset(new TextButton("Drop"));
    synthLabel.reset(new Label("SynthLabel", "<empty>"));
    psLabel.reset(new Label("PitchShiftLabel", "<empty>"));
    synthBtn->addMouseListener(this, true);
    psBtn->addMouseListener(this, true);
    noteButton->addMouseListener(this, true);
    dragBtn->addMouseListener(this, true);

    addAndMakeVisible(numParameterSlot);
    numParameterSlot.setMultiLine(false);
    numParameterSlot.setReturnKeyStartsNewLine(false);
    numParameterSlot.setReadOnly(false);
    numParameterSlot.setInputRestrictions(4, "0123456789");
    numParameterSlot.setScrollbarsShown(false);
    numParameterSlot.setCaretVisible(true);
    numParameterSlot.setPopupMenuEnabled(false);
    numParameterSlot.setText(String(p.getParameterSlotNumber()));
    numParameterSlot.onTextChange = [&]()
    {
        appProperties.getUserSettings()->setValue("parameterSlotNumber", numParameterSlot.getText().getIntValue());
        appProperties.saveIfNeeded();
    };

    transportLabel.setText("PlayHead", dontSendNotification);
    addAndMakeVisible(transportLabel);

    ccLearnBtn->onClick = [this]()
    {
        if (this->ccLearnBtn->getToggleState())
            this->psBundle->startCcLearn();
        else
            this->psBundle->stopCcLearn();
    };

    numInstancesLabel.setText("NumInst", dontSendNotification);
    addAndMakeVisible(numInstancesLabel);
    addAndMakeVisible(numInstancesBox);
    for (int i = 1; i <= MicroChromoAudioProcessor::MAX_INSTANCES; i++)
        numInstancesBox.addItem(String(i), i);
    numInstancesBox.setSelectedId(processor.getNumInstances());
    numInstancesBox.onChange = [&]() {
        if (!ignoreInitialChange)
        {
            synthBundle->closeAllWindows();
            psBundle->closeAllWindows();
            processor.adjustInstanceNumber(numInstancesBox.getSelectedId());
        }
        ignoreInitialChange = false;
    };

    commandManager.setFirstCommandTarget(this);
    setApplicationCommandManagerToWatch(&commandManager);
    commandManager.registerAllCommandsForTarget(this);
    addKeyListener(commandManager.getKeyMappings());

    pluginSortMethod = (KnownPluginList::SortMethod)(appProperties.getUserSettings()->getIntValue("pluginSortMethod", KnownPluginList::sortByManufacturer));
    knownPluginList.addChangeListener(this);

    synthBundle->addChangeListener(this);
    psBundle->addChangeListener(this);
    synthBundle->sendChangeMessage();
    psBundle->sendChangeMessage();

    //lnf.setColour(foleys::LevelMeter::lmMeterGradientLowColour, juce::Colours::green);
    //meterInput.setLookAndFeel(&lnf);
    //meterInput.setMeterSource(&processor.getInputMeterSource());
    //meterOutput.setLookAndFeel(&lnf);
    //meterOutput.setMeterSource(&processor.getOutputMeterSource());

    addAndMakeVisible(menuBar.get());
    addAndMakeVisible(synthBtn.get());
    addAndMakeVisible(psBtn.get());
    addAndMakeVisible(ccLearnBtn.get());
    addAndMakeVisible(dragBtn.get());
    addAndMakeVisible(noteButton.get());
    addAndMakeVisible(synthLabel.get());
    addAndMakeVisible(psLabel.get());
    //addAndMakeVisible(meterInput);
    //addAndMakeVisible(meterOutput);

    changeListenerCallback(synthBundle.get());
    changeListenerCallback(psBundle.get());

    setSize(400, 300);
}

MicroChromoAudioProcessorEditor::~MicroChromoAudioProcessorEditor()
{
    knownPluginList.removeChangeListener(this);
    synthBtn->removeMouseListener(this);
    psBtn->removeMouseListener(this);
    noteButton->removeMouseListener(this);
    ccLearnBtn->removeMouseListener(this);
    dragBtn->removeMouseListener(this);

    synthBundle->removeChangeListener(this);
    psBundle->removeChangeListener(this);

#if JUCE_MAC
    MenuBarModel::setMacMainMenu(nullptr);
#endif
    menuBar = nullptr;

    synthBtn = nullptr;
    psBtn = nullptr;
    synthLabel = nullptr;
    psLabel = nullptr;

    //meterInput.setLookAndFeel(nullptr);
    //meterOutput.setLookAndFeel(nullptr);
}

//==============================================================================
void MicroChromoAudioProcessorEditor::mouseDown(const MouseEvent& e)
{
    auto common = [this](int r, bool isSynth) {
        auto bundle = isSynth ? this->synthBundle : this->psBundle;
        switch (r)
        {
        case 1:  bundle->showRepresentativeWindow(); break;
        case 2:  bundle->showTwoWindows(); break;
        case 3:  bundle->showAllWindows(); break;
        case 4:  bundle->closeAllWindows(); break;
        case 5:  bundle->showWindow(PluginWindow::Type::programs); break;
        case 6:  bundle->showWindow(PluginWindow::Type::generic); break;
        case 7:  bundle->showWindow(PluginWindow::Type::debug); break;
        case 8:  bundle->propagateState(); break;
        case 9:  bundle->openParameterLinkEditor(); break;
        default:
        {
            bundle->closeAllWindows();
            auto types = knownPluginList.getTypes();
            int result = KnownPluginList::getIndexChosenByMenu(types, r);
            auto& desc = types.getReference(result);
            processor.addPlugin(desc, isSynth);
        }
        }
    };
    if (e.eventComponent == synthBtn.get())
    {
        showPopupMenu(0, e.position.toInt(), [this, common](int r) { common(r, true); });
    }
    else if (e.eventComponent == psBtn.get())
    {
        showPopupMenu(0, e.position.toInt(), [this, common](int r) { common(r, false); });
    }
    else if (e.eventComponent == noteButton.get())
    {
        if (isTimerRunning())
            stopTimer();
        else
            startTimerHz(10);
    }
}

void MicroChromoAudioProcessorEditor::mouseDrag(const MouseEvent& e)
{
    if (e.eventComponent == dragBtn.get())
    {
        DragAndDropContainer* dragContainer = DragAndDropContainer::findParentDragContainerFor(dragBtn.get());
        if (!dragContainer->isDragAndDropActive())
        {
            std::shared_ptr<TemporaryFile> outf = std::make_shared<TemporaryFile>();
            if (std::unique_ptr<FileOutputStream> p_os{ outf->getFile().createOutputStream() })
            {
                p_os->writeString("This is a test.");
                DBG("drag clip name " << outf->getFile().getFullPathName());
                DragAndDropContainer::performExternalDragDropOfFiles({ outf->getFile().getFullPathName() }, false, nullptr,
                    [=]() {
                        DBG("Dropped");
                        outf->deleteTemporaryFile();
                    });
            }
        }
    }
}

void MicroChromoAudioProcessorEditor::timerCallback()
{
    if (test)
    {
        MidiMessage controlNote = MidiMessage::controllerEvent(1, 100, Random().nextInt(101) - 50);
        controlNote.setTimeStamp(Time::getMillisecondCounterHiRes() * 0.001);
        lastNote = Random().nextInt(40) + 40;
        MidiMessage startNote = MidiMessage::noteOn(1, lastNote, (uint8)90);
        startNote.setTimeStamp(Time::getMillisecondCounterHiRes() * 0.001);
        psBundle->getCollectorAt(0)->addMessageToQueue(controlNote);
        synthBundle->getCollectorAt(0)->addMessageToQueue(startNote);

        if (processor.getNumInstances() > 1)
        {
            controlNote = MidiMessage::controllerEvent(1, 100, Random().nextInt(101) - 50);
            controlNote.setTimeStamp(Time::getMillisecondCounterHiRes() * 0.001);
            lastNote2 = Random().nextInt(40) + 40;
            startNote = MidiMessage::noteOn(1, lastNote2, (uint8)90);
            startNote.setTimeStamp(Time::getMillisecondCounterHiRes() * 0.001);
            psBundle->getCollectorAt(1)->addMessageToQueue(controlNote);
            synthBundle->getCollectorAt(1)->addMessageToQueue(startNote);
        }
    }
    else
    {
        MidiMessage stopNote = MidiMessage::noteOff(1, lastNote);
        stopNote.setTimeStamp(Time::getMillisecondCounterHiRes() * 0.001);
        synthBundle->getCollectorAt(0)->addMessageToQueue(stopNote);

        if (processor.getNumInstances() > 1)
        {
            stopNote = MidiMessage::noteOff(1, lastNote2);
            stopNote.setTimeStamp(Time::getMillisecondCounterHiRes() * 0.001);
            synthBundle->getCollectorAt(1)->addMessageToQueue(stopNote);
        }
    }
    test = !test;

    transportLabel.setText(String(processor.getInfo().timeInSeconds, 15), dontSendNotification);
}

void MicroChromoAudioProcessorEditor::changeListenerCallback(ChangeBroadcaster* changed)
{
    if (changed == &knownPluginList)
    {
        if (auto savedPluginList = std::unique_ptr<XmlElement>(knownPluginList.createXml()))
        {
            appProperties.getUserSettings()->setValue("pluginList", savedPluginList.get());
            appProperties.saveIfNeeded();
        }
    }
    else if (changed == synthBundle.get())
    {
        if (synthBundle->isLoading())
            synthLabel->setText("Loading...", NotificationType::dontSendNotification);
        else if (synthBundle->isLoaded())
            synthLabel->setText(synthBundle->getName(), NotificationType::dontSendNotification);
    }
    else if (changed == psBundle.get())
    {
        if (psBundle->isLoading())
            psLabel->setText("Loading...", NotificationType::dontSendNotification);
        else if (psBundle->isLoaded())
            psLabel->setText(psBundle->getName(), NotificationType::dontSendNotification);
    }
}

//==============================================================================
StringArray MicroChromoAudioProcessorEditor::getMenuBarNames()
{
    return { "File" };
}

PopupMenu MicroChromoAudioProcessorEditor::getMenuForIndex(int menuIndex, const String& /*menuName*/)
{
    PopupMenu menu;

    if (menuIndex == 0)
    {
        menu.addCommandItem(&commandManager, CommandIDs::openPluginScanner);
    }

    return menu;
}

void MicroChromoAudioProcessorEditor::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
}

//==============================================================================
ApplicationCommandTarget* MicroChromoAudioProcessorEditor::getNextCommandTarget()
{
    return findFirstTargetParentComponent();
}

void MicroChromoAudioProcessorEditor::getAllCommands(Array<CommandID>& c)
{
    Array<CommandID> commands{
        CommandIDs::openPluginScanner
    };
    c.addArray(commands);
}

void MicroChromoAudioProcessorEditor::getCommandInfo(CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID)
    {
    case CommandIDs::openPluginScanner:
        result.setInfo("Open Scanner", "Open the scanner for plugins", "File", 0);
#if JUCE_MAC
        result.addDefaultKeypress('q', ModifierKeys::commandModifier);
#else
        result.addDefaultKeypress('q', ModifierKeys::ctrlModifier);
#endif
        break;
    default:
        break;
    }
}
bool MicroChromoAudioProcessorEditor::perform(const InvocationInfo& info)
{
    switch (info.commandID)
    {
    case CommandIDs::openPluginScanner:
    {
        if (pluginListWindow == nullptr)
            pluginListWindow.reset(new PluginListWindow(*this, formatManager));
        pluginListWindow->toFront(true);
        break;
    }
    default:
        return false;
    }
    return true;
}

//==============================================================================
void MicroChromoAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void MicroChromoAudioProcessorEditor::resized()
{
    auto b = getLocalBounds();
#if !JUCE_MAC
    menuBar->setBounds(b.removeFromTop(LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight()));
#endif
    b.reduce(10, 10);

    auto tmp = b.removeFromTop(40);
    synthBtn->setBounds(tmp.removeFromLeft(100));
    tmp.removeFromLeft(10);
    synthLabel->setBounds(tmp.removeFromLeft(100));

    noteButton->setBounds(tmp.withTrimmedLeft(50).removeFromLeft(100));

    b.removeFromTop(10);

    tmp = b.removeFromTop(40);
    psBtn->setBounds(tmp.removeFromLeft(100));
    tmp.removeFromLeft(10);
    psLabel->setBounds(tmp.removeFromLeft(100));
    ccLearnBtn->setBounds(tmp.withTrimmedLeft(50).removeFromLeft(100));

    b.removeFromTop(10);
    numParameterSlot.setBounds(b.removeFromLeft(100));
    //meterInput.setBounds(b.removeFromLeft(100));
    b.removeFromLeft(10);
    b.removeFromLeft(100);
    //meterOutput.setBounds(b.removeFromLeft(100));

    b.removeFromLeft(10);
    dragBtn->setBounds(b.removeFromTop(40));
    b.removeFromTop(10);
    tmp = b.removeFromTop(30);
    numInstancesLabel.setBounds(tmp.removeFromLeft(60));
    numInstancesBox.setBounds(tmp);

    transportLabel.setBounds(b);
}

void MicroChromoAudioProcessorEditor::buttonClicked(Button* btn)
{
}

void MicroChromoAudioProcessorEditor::showPopupMenu(int type, Point<int> position, std::function<void(int)> callback)
{
    floatMenu.reset(new PopupMenu);

    floatMenu->addItem(1, "Show main plugin GUI");
    floatMenu->addItem(2, "Show two plugin GUI", processor.getNumInstances() > 1);
    floatMenu->addItem(3, "Show all plugin GUI", processor.getNumInstances() > 2);
    floatMenu->addItem(4, "Close all window");
    floatMenu->addItem(5, "Show programs");
    floatMenu->addItem(6, "Show parameters");
    floatMenu->addItem(7, "Show debug log");
    floatMenu->addSeparator();
    floatMenu->addItem(8, "Propagate state to duplicates");
    floatMenu->addItem(9, "Expose parameters");
    floatMenu->addSeparator();
    KnownPluginList::addToMenu(*floatMenu, knownPluginList.getTypes(), pluginSortMethod);
    floatMenu->showMenuAsync({}, ModalCallbackFunction::create([this, callback](int r)
        {
            if (r > 0)
                callback(r);
        }));
}
