/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"

#include "PluginProcessor.h"
#include "PluginInstance.h"
#include "PluginBundle.h"
#include "MainEditor.h"
#include "SimpleMidiEditor.h"
#include "ColorEditor.h"

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
MicroChromoAudioProcessorEditor::CustomTabbedComponent::CustomTabbedComponent(TabbedButtonBar::Orientation orientation, MicroChromoAudioProcessorEditor& owner) :
    TabbedComponent(orientation), _owner(owner) {}

void MicroChromoAudioProcessorEditor::CustomTabbedComponent::currentTabChanged(int newCurrentTabIndex, const String& /*newCurrentTabName*/)
{
    switch (newCurrentTabIndex)
    {
    case 0: _owner.setSize(400, 300 + 90); break;
    case 1: _owner.setSize(500, 500); break;
    case 2: _owner.setSize(300, 500); break;
    default:
        break;
    }
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

    menuBar.reset(new MenuBarComponent(this));
    addAndMakeVisible(menuBar.get());
    menuBar->setVisible(true);

    addAndMakeVisible(playTransportBtn);
    playTransportBtn.onClick = [&]() {
        processor.togglePlayback();
    };

    addAndMakeVisible(stopTransportBtn);
    stopTransportBtn.onClick = [&]() {
        processor.stopPlayback();
    };

    addAndMakeVisible(transportSlider);
    transportSlider.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
    transportSlider.setRange(0.0, 1.0, 0.00001);
    transportSlider.onDragStart = [&]() {
        transportSliderDragging = true;
    };
    transportSlider.onDragEnd = [&]() {
        transportSliderDragging = false;
        processor.setTimeForPlayback(processor.getMidiSequenceEndTime() * transportSlider.getValue());
    };

    transportLabel.setText("00:00:00.000", dontSendNotification);
    transportLabel.setFont(Font(30));
    addAndMakeVisible(transportLabel);

    mainEditor.reset(new MainEditor(p, *this));
    midiEditor.reset(new SimpleMidiEditor(*this));
    colorEditor.reset(new ColorEditor(*this, *midiEditor));

    tabComp.reset(new CustomTabbedComponent(TabbedButtonBar::Orientation::TabsAtTop, *this));
    tabComp->addTab("Main", Colours::darkgrey, mainEditor.get(), false);
    tabComp->addTab("MIDI", Colours::darkgrey, midiEditor.get(), false);
    tabComp->addTab("Color", Colours::darkgrey, colorEditor.get(), false);
    addAndMakeVisible(tabComp.get());

    synthBundle->addChangeListener(this);
    psBundle->addChangeListener(this);

    changeListenerCallback(synthBundle.get());
    changeListenerCallback(psBundle.get());

    setSize(400, 300 + 90);

    startTimer(100);
}

MicroChromoAudioProcessorEditor::~MicroChromoAudioProcessorEditor()
{
    stopTimer();
    knownPluginList.removeChangeListener(this);

    synthBundle->removeChangeListener(this);
    psBundle->removeChangeListener(this);

    menuBar = nullptr;
    mainEditor = nullptr;
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
}

void MicroChromoAudioProcessorEditor::timerCallback()
{
    switch (processor.getTransportState())
    {
    case PLUGIN_PLAYING:
    {
        playTransportBtn.setEnabled(true);
        stopTransportBtn.setEnabled(true);
        transportSlider.setEnabled(true);
        playTransportBtn.setButtonText("Pause");
        break;
    }
    case PLUGIN_PAUSED: case PLUGIN_STOPPED:
    {
        playTransportBtn.setEnabled(true);
        stopTransportBtn.setEnabled(true);
        transportSlider.setEnabled(true);
        playTransportBtn.setButtonText("Play");
        break;
    }
    default:
    {
        playTransportBtn.setEnabled(false);
        stopTransportBtn.setEnabled(false);
        transportSlider.setEnabled(false);
        break;
    }
    }

    auto seconds = processor.getTimeElapsed();
    if (!transportSliderDragging)
        transportSlider.setValue(seconds / processor.getMidiSequenceEndTime(), dontSendNotification);

    std::ostringstream ss;
    ss << std::setfill('0') << std::setw(2) << int(floor(seconds / 3600)) % 24
        << std::setw(1) << ":" << std::setw(2) << int(floor(seconds / 60)) % 60
        << std::setw(1) << ":" << std::setw(2) << int(floor(seconds)) % 60
        << std::setw(1) << "." << std::setw(3) << int(floor(seconds * 1000)) % 1000;
    transportLabel.setText(ss.str(), dontSendNotification);
}

void MicroChromoAudioProcessorEditor::updateMidiEditor()
{
    midiEditor->updateTableContent();
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
        case SLOT_MENU_SHOW_MAIN_GUI:  bundle->showWindow(); break;
        case SLOT_MENU_SHOW_TWO_GUI:  bundle->showWindow(2); break;
        case SLOT_MENU_SHOW_ALL_GUI:  bundle->showWindow(processor.getNumInstances()); break;
        case SLOT_MENU_CLOSE_ALL_GUI:  bundle->closeAllWindows(); break;
        case SLOT_MENU_SHOW_PROGRAMS:  bundle->showWindow(1, PluginWindow::Type::programs); break;
        case SLOT_MENU_SHOW_PARAMETERS:  bundle->showWindow(1, PluginWindow::Type::generic); break;
        case SLOT_MENU_SHOW_DEBUG_LOG:  bundle->showWindow(1, PluginWindow::Type::debug); break;
        case SLOT_MENU_PROPAGATE_STATE:  bundle->propagateState(); break;
        case SLOT_MENU_EXPOSE_PARAMETER:  bundle->openParameterLinkEditor(); break;
        case SLOT_MENU_START_CC: bundle->getCcLearnModule().startLearning(); break;
        case SLOT_MENU_MANAGE_CC: bundle->getCcLearnModule().showStatus(); break;
        case SLOT_MENU_CLEAR_CC: bundle->getCcLearnModule().reset(true); break;
        case SLOT_MENU_USE_KONTAKT: processor.toggleUseKontakt(processor.getPitchShiftModulationSource() != USE_KONTAKT); break;
        case SLOT_MENU_COPY_KONTAKT_SCRIPT: 
            SystemClipboard::copyTextToClipboard(String(BinaryData::MicroChromoKontaktScript_txt)
                .replace("@CC_START@", String(processor.getCcBase()))
                .replace("@CC_END@", String(processor.getCcBase() + 11))); 
            break;
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
        default:
        {
            if (menuItemID >= CC_VALUE_BASE && menuItemID < CC_VALUE_BASE + 128 - 12 + 1)
                processor.updateKontaktCcBase(menuItemID - CC_VALUE_BASE);
            break;
        }
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

    auto bottomPanel = b.removeFromBottom(90).withSizeKeepingCentre(300, 90);
    bottomPanel.reduce(5, 10);
    transportSlider.setBounds(bottomPanel.removeFromTop(24));
    bottomPanel.removeFromTop(6);
    playTransportBtn.setBounds(bottomPanel.removeFromLeft(50));
    bottomPanel.removeFromLeft(6);
    stopTransportBtn.setBounds(bottomPanel.removeFromLeft(50));
    bottomPanel.removeFromLeft(6);
    transportLabel.setBounds(bottomPanel.withSizeKeepingCentre(jmin(bottomPanel.getWidth(), 
        6 + transportLabel.getFont().getStringWidth(transportLabel.getText())), bottomPanel.getHeight()));

    //6 + label.getFont().getStringWidth(label.getText()), 16)

    menuBar->setBounds(b.removeFromTop(LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight()));
    tabComp->setBounds(b);
}
