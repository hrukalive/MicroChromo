/*
 * This file is part of the MicroChromo distribution
 * (https://github.com/hrukalive/MicroChromo).
 * Copyright (c) 2020 UIUC.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "PluginEditor.h"

#include "PluginProcessor.h"
#include "PluginInstance.h"
#include "PluginBundle.h"

#include "ParameterCcLearn.h"

#include "MainEditor.h"
#include "ColorEditor.h"
#include "SimpleNoteEditor.h"
#include "SimpleTimeEditor.h"
#include "TuningEditor.h"

//__________________________________________________________________________
//                                                                          |\
// PluginListWindow                                                         | |
//__________________________________________________________________________| |
//___________________________________________________________________________\|

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

//__________________________________________________________________________
//                                                                          |\
// CustomTabbedComponent                                                    | |
//__________________________________________________________________________| |
//___________________________________________________________________________\|

MicroChromoAudioProcessorEditor::CustomTabbedComponent::CustomTabbedComponent(TabbedButtonBar::Orientation orientation, MicroChromoAudioProcessorEditor& owner) :
    TabbedComponent(orientation), _owner(owner) {}

void MicroChromoAudioProcessorEditor::CustomTabbedComponent::currentTabChanged(int newCurrentTabIndex, const String& /*newCurrentTabName*/)
{
    switch (newCurrentTabIndex)
    {
    case 0: _owner.setSize(400, 300 + 90); break;
    case 1: _owner.setSize(570, 580); break;
    case 2: _owner.setSize(350, 580); break;
    case 3: _owner.setSize(490, 580); break;
    case 4: _owner.setSize(350, 580); break;
    default:
        break;
    }
}

//__________________________________________________________________________
//                                                                          |\
// MicroChromoAudioProcessorEditor                                          | |
//__________________________________________________________________________| |
//___________________________________________________________________________\|

MicroChromoAudioProcessorEditor::MicroChromoAudioProcessorEditor (MicroChromoAudioProcessor& p)
    : AudioProcessorEditor (&p), 
    processor (p), 
    appProperties(p.getApplicationProperties()),
    knownPluginList(p.getKnownPluginList()),
    formatManager(p.getAudioPluginFormatManager()),
    undoManager(p.getUndoManager()),
    synthBundle(p.getSynthBundlePtr()),
    psBundle(p.getPSBundlePtr())
{
    commandManager.setFirstCommandTarget(this);
    setApplicationCommandManagerToWatch(&commandManager);
    commandManager.registerAllCommandsForTarget(this);
    addKeyListener(commandManager.getKeyMappings());
    setWantsKeyboardFocus(true);

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

    addAndMakeVisible(panicBtn);
    panicBtn.onClick = [&]() {
        processor.triggerPanic();
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
    noteEditor.reset(new SimpleNoteEditor(*this));
    timeEditor.reset(new SimpleTimeEditor(*this));
    colorEditor.reset(new ColorEditor(*this));
    tuningEditor.reset(new TuningEditor(*this));


    tabComp.reset(new CustomTabbedComponent(TabbedButtonBar::Orientation::TabsAtTop, *this));
    tabComp->addTab("Main", Colours::darkgrey, mainEditor.get(), false);
    tabComp->addTab("MIDI", Colours::darkgrey, noteEditor.get(), false);
    tabComp->addTab("Time", Colours::darkgrey, timeEditor.get(), false);
    tabComp->addTab("Color", Colours::darkgrey, colorEditor.get(), false);
    tabComp->addTab("Tuning", Colours::darkgrey, tuningEditor.get(), false);
    addAndMakeVisible(tabComp.get());

    synthBundle->addChangeListener(this);
    psBundle->addChangeListener(this);

    processor.getProject().addListener(this);

    changeListenerCallback(synthBundle.get());
    changeListenerCallback(psBundle.get());

    setSize(400, 300 + 90);

    startTimer(100);
}

MicroChromoAudioProcessorEditor::~MicroChromoAudioProcessorEditor()
{
    stopTimer();
    processor.getProject().removeListener(this);

    knownPluginList.removeChangeListener(this);

    synthBundle->removeChangeListener(this);
    psBundle->removeChangeListener(this);

    menuBar = nullptr;
    mainEditor = nullptr;
}

//===------------------------------------------------------------------===//
// ApplicationCommandTarget
//===------------------------------------------------------------------===//
ApplicationCommandTarget* MicroChromoAudioProcessorEditor::getNextCommandTarget()
{
    return findFirstTargetParentComponent();
}

void MicroChromoAudioProcessorEditor::getAllCommands(Array<CommandID>& c)
{
    Array<CommandID> commands{
        CommandIDs::openPluginScanner,
        CommandIDs::openProject,
        CommandIDs::saveProject,
        CommandIDs::saveAsProject,
        CommandIDs::importMidi,
        CommandIDs::exportMidi,
        CommandIDs::undoAction,
        CommandIDs::redoAction
    };
    c.addArray(commands);
}

void MicroChromoAudioProcessorEditor::getCommandInfo(CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID)
    {
    case CommandIDs::openProject:
        result.setInfo("Open Project", "Open project", "File", 0);
#if JUCE_MAC
        result.addDefaultKeypress('o', ModifierKeys::commandModifier);
#else
        result.addDefaultKeypress('o', ModifierKeys::ctrlModifier);
#endif
        break;
    case CommandIDs::saveProject:
        result.setInfo("Save Project", "Save project", "File", 0);
#if JUCE_MAC
        result.addDefaultKeypress('s', ModifierKeys::commandModifier);
#else
        result.addDefaultKeypress('s', ModifierKeys::ctrlModifier);
#endif
        break;
    case CommandIDs::saveAsProject:
        result.setInfo("Save Project As", "Save as", "File", 0);
#if JUCE_MAC
        result.addDefaultKeypress('s', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
#else
        result.addDefaultKeypress('s', ModifierKeys::ctrlModifier | ModifierKeys::shiftModifier);
#endif
        break;
    case CommandIDs::importMidi:
        result.setInfo("Import MIDI", "Import MIDI file", "File", 0);
        break;
    case CommandIDs::exportMidi:
        result.setInfo("Export MIDI", "Export MIDI file", "File", 0);
        break;
    case CommandIDs::undoAction:
        result.setInfo("Undo " + undoManager.getUndoDescription(), "Undo", "Edit",
            undoManager.canUndo() ? 0 : ApplicationCommandInfo::CommandFlags::isDisabled);
#if JUCE_MAC
        result.addDefaultKeypress('z', ModifierKeys::commandModifier);
#else
        result.addDefaultKeypress('z', ModifierKeys::ctrlModifier);
#endif
        break;
    case CommandIDs::redoAction:
        result.setInfo("Redo " + undoManager.getRedoDescription(), "Redo", "Edit",
            undoManager.canRedo() ? 0 : ApplicationCommandInfo::CommandFlags::isDisabled);
#if JUCE_MAC
        result.addDefaultKeypress('z', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
#else
        result.addDefaultKeypress('z', ModifierKeys::ctrlModifier | ModifierKeys::shiftModifier);
#endif
        break;
    case CommandIDs::openPluginScanner:
        result.setInfo("Open Scanner", "Open the scanner for plugins", "Plugin", 0);
#if JUCE_MAC
        result.addDefaultKeypress('q', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
#else
        result.addDefaultKeypress('q', ModifierKeys::ctrlModifier | ModifierKeys::shiftModifier);
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
    case CommandIDs::openProject: processor.loadFromUserSpecifiedFile(true); break;
    case CommandIDs::saveProject: processor.setChangedFlag(true); processor.save(true, true); break;
    case CommandIDs::saveAsProject: processor.saveAsInteractive(true); break;
    case CommandIDs::importMidi: mainEditor->dropButton->triggerClick(); break;
    case CommandIDs::exportMidi: mainEditor->exportMidiDialog(); break;
    case CommandIDs::undoAction: undoManager.undo(); break;
    case CommandIDs::redoAction: undoManager.redo(); break;
    default:
        return false;
    }
    return true;
}

//===------------------------------------------------------------------===//
// MenuBarModel
//===------------------------------------------------------------------===//
StringArray MicroChromoAudioProcessorEditor::getMenuBarNames()
{
    return { "File", "Edit", "Synth", "Effect", "Plugin" };
}

PopupMenu MicroChromoAudioProcessorEditor::getMenuForIndex(int menuIndex, const String& /*menuName*/)
{
    PopupMenu menu;

    if (menuIndex == 0)
    {
        menu.addCommandItem(&commandManager, CommandIDs::openProject);
        menu.addCommandItem(&commandManager, CommandIDs::saveProject);
        menu.addCommandItem(&commandManager, CommandIDs::saveAsProject);
        menu.addSeparator();
        menu.addCommandItem(&commandManager, CommandIDs::importMidi);
        menu.addCommandItem(&commandManager, CommandIDs::exportMidi);
    }
    else if (menuIndex == 1)
    {
        menu.addCommandItem(&commandManager, CommandIDs::undoAction);
        menu.addCommandItem(&commandManager, CommandIDs::redoAction);
    }
    else if (menuIndex == 2)
        menu = *synthBundle->getMainPopupMenu();
    else if (menuIndex == 3)
        menu = *psBundle->getMainPopupMenu();
    else if (menuIndex == 4)
    {
        menu.addCommandItem(&commandManager, CommandIDs::openPluginScanner);
        menu.addSeparator();
        menu.addItem(PLUGIN_SORT_MANUFACTURER, "Sort by Manufacturer", true, pluginSortMethod == KnownPluginList::SortMethod::sortByManufacturer);
        menu.addItem(PLUGIN_SORT_CATEGORY, "Sort by Category", true, pluginSortMethod == KnownPluginList::SortMethod::sortByCategory);
        menu.addItem(PLUGIN_SORT_ALPHABETICALLY, "Sort Alphabetically", true, pluginSortMethod == KnownPluginList::SortMethod::sortAlphabetically);
    }

    return menu;
}

void MicroChromoAudioProcessorEditor::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
    if (topLevelMenuIndex == 2 || topLevelMenuIndex == 3)
    {
        bool isSynth = topLevelMenuIndex == 2;
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
        case SLOT_MENU_CLEAR_CC: bundle->getCcLearnModule().resetCcLearn(true); break;
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
    else if (topLevelMenuIndex == 4)
    {
        switch (menuItemID)
        {
        case PLUGIN_SORT_MANUFACTURER: pluginSortMethod = KnownPluginList::SortMethod::sortByManufacturer; break;
        case PLUGIN_SORT_CATEGORY: pluginSortMethod = KnownPluginList::SortMethod::sortByCategory; break;
        case PLUGIN_SORT_ALPHABETICALLY: pluginSortMethod = KnownPluginList::SortMethod::sortAlphabetically; break;
        default: break;
        }
    }
}

//===------------------------------------------------------------------===//
// Keyboard Listeners
//===------------------------------------------------------------------===//
bool MicroChromoAudioProcessorEditor::keyPressed(const KeyPress& key)
{
    if (key.isKeyCode(KeyPress::spaceKey))
    {
        playTransportBtn.triggerClick();
        return true;
    }
    return false;
}

//===------------------------------------------------------------------===//
// Callbacks
//===------------------------------------------------------------------===//
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

//===------------------------------------------------------------------===//
// Project Listener
//===------------------------------------------------------------------===//
void MicroChromoAudioProcessorEditor::onReloadProjectContent(const Array<MidiTrack*>& tracks)
{
    AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::InfoIcon, "Done", "Project loaded.");
}

//===------------------------------------------------------------------===//
// Components
//===------------------------------------------------------------------===//
void MicroChromoAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void MicroChromoAudioProcessorEditor::resized()
{
    auto b = getLocalBounds();

    auto bottomPanel = b.removeFromBottom(90).withSizeKeepingCentre(320, 90);
    bottomPanel.reduce(5, 10);
    transportSlider.setBounds(bottomPanel.removeFromTop(24));
    bottomPanel.removeFromTop(6);
    playTransportBtn.setBounds(bottomPanel.removeFromLeft(50));
    bottomPanel.removeFromLeft(4);
    stopTransportBtn.setBounds(bottomPanel.removeFromLeft(50));
    bottomPanel.removeFromLeft(4);
    panicBtn.setBounds(bottomPanel.removeFromLeft(20));
    bottomPanel.removeFromLeft(4);
    transportLabel.setBounds(bottomPanel.withSizeKeepingCentre(jmin(bottomPanel.getWidth(), 
        6 + transportLabel.getFont().getStringWidth(transportLabel.getText())), bottomPanel.getHeight()));

    menuBar->setBounds(b.removeFromTop(LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight()));
    tabComp->setBounds(b);
}
