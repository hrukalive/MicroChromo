/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MicroChromoAudioProcessorEditor::PluginListWindow::PluginListWindow(MicroChromoAudioProcessorEditor &mw, AudioPluginFormatManager& pluginFormatManager)
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

MicroChromoAudioProcessorEditor::PluginListWindow::~PluginListWindow()
{
    clearContentComponent();
}

void MicroChromoAudioProcessorEditor::PluginListWindow::closeButtonPressed()
{
    owner.pluginListWindow = nullptr;
}

//==============================================================================
MicroChromoAudioProcessorEditor::MainEditor::MainEditor(MicroChromoAudioProcessor& p, MicroChromoAudioProcessorEditor& parent)
    : AudioProcessorEditor(&p), 
    _parent(parent),
    processor(p),
    appProperties(p.getApplicationProperties()),
    synthBundle(p.getSynthBundlePtr()),
    psBundle(p.getPSBundlePtr())
{
    synthButton.reset(new TextButton("Synth"));
    addAndMakeVisible(synthButton.get());
    synthButton->addMouseListener(this, true);
    synthLabel.reset(new Label("SynthLabel", "<empty>"));
    addAndMakeVisible(synthLabel.get());

    effectButton.reset(new TextButton("Effect"));
    addAndMakeVisible(effectButton.get());
    effectButton->addMouseListener(this, true);
    effectLabel.reset(new Label("EffectLabel", "<empty>"));
    addAndMakeVisible(effectLabel.get());

    dragButton.reset(new TextButton("Drop"));
    addAndMakeVisible(dragButton.get());
    dragButton->addMouseListener(this, true);

    noteBtn.reset(new TextButton("Note"));
    noteBtn->addMouseListener(this, true);
    addAndMakeVisible(noteBtn.get());

    numParameterSlot.reset(new TextEditor());
    addAndMakeVisible(numParameterSlot.get());
    numParameterSlot->setMultiLine(false);
    numParameterSlot->setReturnKeyStartsNewLine(false);
    numParameterSlot->setReadOnly(false);
    numParameterSlot->setInputRestrictions(4, "0123456789");
    numParameterSlot->setScrollbarsShown(false);
    numParameterSlot->setCaretVisible(true);
    numParameterSlot->setPopupMenuEnabled(false);
    numParameterSlot->setText(String(p.getParameterSlotNumber()));
    numParameterSlot->onTextChange = [&]()
    {
        appProperties.getUserSettings()->setValue("parameterSlotNumber", numParameterSlot->getText().getIntValue());
        appProperties.saveIfNeeded();
    };
    numParameterLabel.reset(new Label());
    addAndMakeVisible(numParameterLabel.get());
    numParameterLabel->setText("# Parameter Slots", dontSendNotification);

    numInstancesBox.reset(new ComboBox());
    addAndMakeVisible(numInstancesBox.get());
    for (int i = 1; i <= MicroChromoAudioProcessor::MAX_INSTANCES; i++)
        numInstancesBox->addItem(String(i), i);
    numInstancesBox->setSelectedId(processor.getNumInstances());
    numInstancesBox->onChange = [&]() {
        if (!ignoreInitialChange)
        {
            synthBundle->closeAllWindows();
            psBundle->closeAllWindows();
            processor.adjustInstanceNumber(numInstancesBox->getSelectedId());
        }
        ignoreInitialChange = false;
    };
    numInstancesLabel.reset(new Label());
    addAndMakeVisible(numInstancesLabel.get());
    numInstancesLabel->setText("# Instance", dontSendNotification);

    synthBundle->addChangeListener(this);
    psBundle->addChangeListener(this);

    changeListenerCallback(synthBundle.get());
    changeListenerCallback(psBundle.get());

    setResizable(true, false);
    setSize(400, 230);
}

MicroChromoAudioProcessorEditor::MainEditor::~MainEditor()
{
    synthButton->removeMouseListener(this);
    effectButton->removeMouseListener(this);
    dragButton->removeMouseListener(this);
    noteBtn->removeMouseListener(this);
}

void MicroChromoAudioProcessorEditor::MainEditor::changeListenerCallback(ChangeBroadcaster* changed)
{
    if (changed == synthBundle.get())
    {
        if (synthBundle->isLoading())
            synthLabel->setText("Loading...", NotificationType::dontSendNotification);
        else if (synthBundle->isLoaded())
            synthLabel->setText(synthBundle->getName(), NotificationType::dontSendNotification);
    }
    else if (changed == psBundle.get())
    {
        if (psBundle->isLoading())
            effectLabel->setText("Loading...", NotificationType::dontSendNotification);
        else if (psBundle->isLoaded())
            effectLabel->setText(psBundle->getName(), NotificationType::dontSendNotification);
    }
}

void MicroChromoAudioProcessorEditor::MainEditor::mouseDown(const MouseEvent& e)
{
    if (e.eventComponent == synthButton.get())
    {
        floatMenu = synthBundle->getPluginPopupMenu(_parent.getPluginSortMethod(), processor.getSynthKnownPluginList());
        floatMenu->showMenuAsync({}, ModalCallbackFunction::create([this](int r) { bundlePopupMenuSelected(r, true); }));
    }
    else if (e.eventComponent == effectButton.get())
    {
        floatMenu = psBundle->getPluginPopupMenu(_parent.getPluginSortMethod(), processor.getPsKnownPluginList());
        floatMenu->showMenuAsync({}, ModalCallbackFunction::create([this](int r) { bundlePopupMenuSelected(r, false); }));
    }
    else if (e.eventComponent == noteBtn.get())
    {
        if (isTimerRunning())
            stopTimer();
        else
            startTimerHz(10);
    }
}

void MicroChromoAudioProcessorEditor::MainEditor::mouseDrag(const MouseEvent& e)
{
    if (e.eventComponent == dragButton.get())
    {
        DragAndDropContainer* dragContainer = DragAndDropContainer::findParentDragContainerFor(dragButton.get());
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

void MicroChromoAudioProcessorEditor::MainEditor::timerCallback()
{
    if (test)
    {
        MidiMessage controlNote = MidiMessage::controllerEvent(1, 100, Random().nextInt(101) - 50);
        controlNote.setTimeStamp(Time::getMillisecondCounterHiRes() * 0.001);
        lastNote = Random().nextInt(40) + 40;
        MidiMessage startNote = MidiMessage::noteOn(1, lastNote, (uint8)90);
        startNote.setTimeStamp(Time::getMillisecondCounterHiRes() * 0.001);
        psBundle->getCollectorAt(0)->addMessageToQueue(controlNote);
        synthBundle->getCollectorAt(0)->addMessageToQueue(controlNote);
        synthBundle->getCollectorAt(0)->addMessageToQueue(startNote);

        if (processor.getNumInstances() > 1)
        {
            controlNote = MidiMessage::controllerEvent(1, 100, Random().nextInt(101) - 50);
            controlNote.setTimeStamp(Time::getMillisecondCounterHiRes() * 0.001);
            lastNote2 = Random().nextInt(40) + 40;
            startNote = MidiMessage::noteOn(1, lastNote2, (uint8)90);
            startNote.setTimeStamp(Time::getMillisecondCounterHiRes() * 0.001);
            psBundle->getCollectorAt(1)->addMessageToQueue(controlNote);
            synthBundle->getCollectorAt(1)->addMessageToQueue(controlNote);
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
}
void MicroChromoAudioProcessorEditor::MainEditor::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void MicroChromoAudioProcessorEditor::MainEditor::resized()
{
    auto b = getLocalBounds();
    b.reduce(10, 10);
    auto spaceHeightSmall = jmax(1.0f, (b.getHeight() - 170) / 32.0f * 6);
    auto spaceHeightLarge = jmax(1.0f, (b.getHeight() - 170) / 32.0f * 10);

    auto tmp = b.removeFromBottom(30);
    dragButton->setBounds(tmp.removeFromLeft(tmp.proportionOfWidth(0.7)));
    tmp.removeFromLeft(6);
    noteBtn->setBounds(tmp);
    b.removeFromBottom(spaceHeightLarge);

    auto leftPanel = b.removeFromLeft(b.proportionOfWidth(0.3));
    auto rightPanel = b.withTrimmedLeft(10);

    numInstancesLabel->setBounds(leftPanel.removeFromTop(30));
    leftPanel.removeFromTop(spaceHeightSmall);
    numParameterLabel->setBounds(leftPanel.removeFromTop(30));
    leftPanel.removeFromTop(spaceHeightLarge);

    synthButton->setBounds(leftPanel.removeFromTop(40));
    leftPanel.removeFromTop(spaceHeightSmall);
    effectButton->setBounds(leftPanel.removeFromTop(40));

    numInstancesBox->setBounds(rightPanel.removeFromTop(30));
    rightPanel.removeFromTop(spaceHeightSmall);
    numParameterSlot->setBounds(rightPanel.removeFromTop(30));
    rightPanel.removeFromTop(spaceHeightLarge);

    synthLabel->setBounds(rightPanel.removeFromTop(40));
    rightPanel.removeFromTop(spaceHeightSmall);
    effectLabel->setBounds(rightPanel.removeFromTop(40));
}

//==============================================================================
MicroChromoAudioProcessorEditor::CustomTabbedButtonComponent::CustomTabbedButtonComponent(MicroChromoAudioProcessorEditor& parent, TabbedButtonBar::Orientation orientation) :
    TabbedButtonBar(orientation), _parent(parent) {}

void MicroChromoAudioProcessorEditor::CustomTabbedButtonComponent::currentTabChanged(int newCurrentTabIndex, const String&)
{
    _parent.currentTabChanged(newCurrentTabIndex);
}

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
    commandManager.setFirstCommandTarget(this);
    setApplicationCommandManagerToWatch(&commandManager);
    commandManager.registerAllCommandsForTarget(this);
    addKeyListener(commandManager.getKeyMappings());

    pluginSortMethod = (KnownPluginList::SortMethod)(appProperties.getUserSettings()->getIntValue("pluginSortMethod", KnownPluginList::sortByManufacturer));
    knownPluginList.addChangeListener(this);

    mainEditor.reset(new MainEditor(p, *this));

    menuBar.reset(new MenuBarComponent(this));
    addAndMakeVisible(menuBar.get());
    menuBar->setVisible(true);

    tabbedButtons.reset(new CustomTabbedButtonComponent(*this, TabbedButtonBar::Orientation::TabsAtTop));
    addAndMakeVisible(tabbedButtons.get());

    centerMessageLabel.reset(new Label());
    addAndMakeVisible(centerMessageLabel.get());
    centerMessageLabel->setText("MicroChromo", dontSendNotification);
    centerMessageLabel->setVisible(true);

    mainComp.reset(new MainViewport(3));
    addAndMakeVisible(mainComp.get());
    mainComp->setUI(0, mainEditor.get());

    tabbedButtons->addTab("Main", Colours::lightgrey, 0);
    tabbedButtons->addTab("Synth - Empty", Colours::lightgrey, 1);
    tabbedButtons->addTab("Effect - Empty", Colours::lightgrey, 2);

    synthBundle->addChangeListener(this);
    psBundle->addChangeListener(this);

    changeListenerCallback(synthBundle.get());
    changeListenerCallback(psBundle.get());

    setResizable(true, true);
    setSize(400, 300);
    tabbedButtons->setCurrentTabIndex(0);
}

MicroChromoAudioProcessorEditor::~MicroChromoAudioProcessorEditor()
{
    knownPluginList.removeChangeListener(this);

    synthBundle->removeChangeListener(this);
    psBundle->removeChangeListener(this);

    menuBar = nullptr;
    tabbedButtons = nullptr;
    synthUi = nullptr;
    effectUi = nullptr;
    mainEditor = nullptr;
    mainComp = nullptr;
}

//==============================================================================

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
    else
    {
        if (changed == synthBundle.get())
        {
            if (synthBundle->isLoading())
            {
                centerMessageLabel->setText("Loading", dontSendNotification);
                mainComp->setUI(1, nullptr);
                synthBundle->getMainProcessor()->processor->editorBeingDeleted(synthUi.get());
                synthUi = nullptr;
                tabbedButtons->setTabName(1, "Synth - Loading");
            }
            else if (synthBundle->isLoaded())
            {
                if (auto* ui = PluginWindow::createProcessorEditor(*synthBundle->getMainProcessor()->processor, PluginWindow::Type::normal))
                {
                    centerMessageLabel->setText("Done", dontSendNotification);
                    synthUi.reset(ui);
                    mainComp->setUI(1, synthUi.get());
                    synthWidth = synthUi->getConstrainer()->getMinimumWidth();
                    synthHeight = synthUi->getConstrainer()->getMinimumHeight();
                    tabbedButtons->setTabName(1, "Synth - " + synthBundle->getName());
                }
                else
                {
                    centerMessageLabel->setText("No synth loaded", dontSendNotification);
                    tabbedButtons->setTabName(1, "Synth - Failed");
                }
            }
            else
            {
                mainComp->setUI(1, nullptr);
                centerMessageLabel->setText("No synth loaded", dontSendNotification);
                tabbedButtons->setTabName(1, "Synth - Empty");
            }
        }
        else if (changed == psBundle.get())
        {
            if (psBundle->isLoading())
            {
                centerMessageLabel->setText("Loading", dontSendNotification);
                mainComp->setUI(2, nullptr);
                psBundle->getMainProcessor()->processor->editorBeingDeleted(effectUi.get());
                effectUi = nullptr;
                tabbedButtons->setTabName(2, "Effect - Loading");
            }
            else if (psBundle->isLoaded())
            {
                if (auto* ui = PluginWindow::createProcessorEditor(*psBundle->getMainProcessor()->processor, PluginWindow::Type::normal))
                {
                    centerMessageLabel->setText("Done", dontSendNotification);
                    effectUi.reset(ui);
                    mainComp->setUI(2, effectUi.get());
                    effectWidth = effectUi->getConstrainer()->getMinimumWidth();
                    effectHeight = effectUi->getConstrainer()->getMinimumHeight();
                    tabbedButtons->setTabName(2, "Effect - " + psBundle->getName());
                }
                else
                {
                    centerMessageLabel->setText("No effect loaded", dontSendNotification);
                    tabbedButtons->setTabName(2, "Effect - Failed");
                }
            }
            else
            {
                mainComp->setUI(2, nullptr);
                centerMessageLabel->setText("No effect loaded", dontSendNotification);
                tabbedButtons->setTabName(2, "Effect - Empty");
            }
        }
    }
}

void MicroChromoAudioProcessorEditor::focusGained(FocusChangeType cause)
{
    if (cause == FocusChangeType::focusChangedByMouseClick)
    {
        synthBundle->bringToFront();
        psBundle->bringToFront();
    }
}

//==============================================================================
StringArray MicroChromoAudioProcessorEditor::getMenuBarNames()
{
    return { "File", "Synth", "Effect" };
}

PopupMenu MicroChromoAudioProcessorEditor::getMenuForIndex(int menuIndex, const String& /*menuName*/)
{
    PopupMenu menu;

    if (menuIndex == 0)
    {
        menu.addCommandItem(&commandManager, CommandIDs::openPluginScanner);
        menu.addSeparator();
        menu.addItem(PLUGIN_SORT_MANUFACTURER, "Sort by Manufacturer", true, pluginSortMethod == KnownPluginList::SortMethod::sortByManufacturer);
        menu.addItem(PLUGIN_SORT_CATEGORY, "Sort by Category", true, pluginSortMethod == KnownPluginList::SortMethod::sortByCategory);
        menu.addItem(PLUGIN_SORT_ALPHABETICALLY, "Sort Alphabetically", true, pluginSortMethod == KnownPluginList::SortMethod::sortAlphabetically);
    }
    else if (menuIndex == 1)
        menu = *synthBundle->getMainPopupMenu();
    else if (menuIndex == 2)
        menu = *psBundle->getMainPopupMenu();

    return menu;
}

void MicroChromoAudioProcessorEditor::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
    if (topLevelMenuIndex == 0)
    {
        switch (menuItemID)
        {
        case PLUGIN_SORT_MANUFACTURER: pluginSortMethod = KnownPluginList::SortMethod::sortByManufacturer; break;
        case PLUGIN_SORT_CATEGORY: pluginSortMethod = KnownPluginList::SortMethod::sortByCategory; break;
        case PLUGIN_SORT_ALPHABETICALLY: pluginSortMethod = KnownPluginList::SortMethod::sortAlphabetically; break;
        default: break;
        }
    }
    else if (topLevelMenuIndex == 1 || topLevelMenuIndex == 2)
    {
        bool isSynth = topLevelMenuIndex == 1;
        auto bundle = isSynth ? this->synthBundle : this->psBundle;
        switch (menuItemID)
        {
        case SLOT_MENU_SHOW_TWO_GUI:  bundle->showTwoWindows(); break;
        case SLOT_MENU_SHOW_ALL_GUI:  bundle->showAllWindows(); break;
        case SLOT_MENU_CLOSE_ALL_GUI:  bundle->closeAllWindows(); break;
        case SLOT_MENU_SHOW_PROGRAMS:  bundle->showWindow(PluginWindow::Type::programs); break;
        case SLOT_MENU_SHOW_PARAMETERS:  bundle->showWindow(PluginWindow::Type::generic); break;
        case SLOT_MENU_SHOW_DEBUG_LOG:  bundle->showWindow(PluginWindow::Type::debug); break;
        case SLOT_MENU_PROPAGATE_STATE:  bundle->propagateState(); break;
        case SLOT_MENU_EXPOSE_PARAMETER:  bundle->openParameterLinkEditor(); break;
        case SLOT_MENU_START_CC: bundle->getCcLearnModule().startLearning(); break;
        case SLOT_MENU_SHOW_CC: bundle->getCcLearnModule().showStatus(); break;
        case SLOT_MENU_CLEAR_CC: bundle->getCcLearnModule().reset(); break;
        case SLOT_MENU_LOAD_EMPTY_PLUGIN:
        {
            bundle->closeAllWindows();
            processor.addPlugin(bundle->getEmptyPluginDescription(), isSynth);
            break;
        }
        case SLOT_MENU_LOAD_DEFAULT_PLUGIN:
        {
            bundle->closeAllWindows();
            processor.addPlugin(bundle->getDefaultPluginDescription(), isSynth);
            break;
        }
        default: break;
        }
    }
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
    menuBar->setBounds(b.removeFromTop(LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight()));
    tabbedButtons->setBounds(b.removeFromTop(30));
    mainComp->setBounds(b);
    centerMessageLabel->setBounds(b.withSizeKeepingCentre(6 + centerMessageLabel->getFont().getStringWidth(centerMessageLabel->getText()), 16));
}

void MicroChromoAudioProcessorEditor::currentTabChanged(int newCurrentTabIndex)
{
    if (newCurrentTabIndex == 0)
    {
        mainComp->showUI(0);
        setResizable(true, true);
        setSize(400, 300);
    }
    else if (newCurrentTabIndex == 1)
    {
        mainComp->showUI(1);
        setSize(synthWidth, synthHeight + LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight() + 30 + 4);
        mainComp->showUI(2);
        setSize(effectWidth, effectHeight + LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight() + 30 + 4);
        mainComp->showUI(1);
        setSize(synthWidth, synthHeight + LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight() + 30 + 4);
    }
    else if (newCurrentTabIndex == 2)
    {
        mainComp->showUI(2);
        setSize(effectWidth, effectHeight + LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight() + 30 + 4);
        mainComp->showUI(1);
        setSize(synthWidth, synthHeight + LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight() + 30 + 4);
        mainComp->showUI(2);
        setSize(effectWidth, effectHeight + LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight() + 30 + 4);
    }
}
