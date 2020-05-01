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

#pragma once

#include "Common.h"
#include "ProjectListener.h"

class MicroChromoAudioProcessor;
class PluginBundle;
class MainEditor;
class ColorEditor;
class SimpleNoteEditor;
class SimpleTimeEditor;
class TuningEditor;

//==============================================================================
class MicroChromoAudioProcessorEditor  : 
    public AudioProcessorEditor,
    public ApplicationCommandTarget,
    public ProjectListener,
    public MenuBarModel,
    public ChangeListener,
    public Timer
{
public:
    MicroChromoAudioProcessorEditor (MicroChromoAudioProcessor&);
    ~MicroChromoAudioProcessorEditor();

    //===------------------------------------------------------------------===//
    // Components
    //===------------------------------------------------------------------===//
    void paint(Graphics&) override;
    void resized() override;

    //===------------------------------------------------------------------===//
    // ApplicationCommandTarget
    //===------------------------------------------------------------------===//
    ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands(Array<CommandID>& c) override;
    void getCommandInfo(CommandID commandID, ApplicationCommandInfo& result) override;
    bool perform(const InvocationInfo& info) override;

    //===------------------------------------------------------------------===//
    // MenuBarModel
    //===------------------------------------------------------------------===//
    StringArray getMenuBarNames() override;
    PopupMenu getMenuForIndex(int menuIndex, const String& /*menuName*/) override;
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

    //===------------------------------------------------------------------===//
    // Accessors
    //===------------------------------------------------------------------===//
    MicroChromoAudioProcessor& getProcessor() { return processor; }
    KnownPluginList::SortMethod getPluginSortMethod() { return pluginSortMethod; }

    //===------------------------------------------------------------------===//
    // Keyboard Listeners
    //===------------------------------------------------------------------===//
    bool keyPressed(const KeyPress& key) override;

    //===------------------------------------------------------------------===//
    // Callbacks
    //===------------------------------------------------------------------===//
    void changeListenerCallback(ChangeBroadcaster*) override;
    void timerCallback() override;

    //===------------------------------------------------------------------===//
    // Project Listener
    //===------------------------------------------------------------------===//
    void onAddMidiEvent(const MidiEvent& event) override {}
    void onPostAddMidiEvent() override {}
    void onChangeMidiEvent(const MidiEvent& oldEvent, const MidiEvent& newEvent) override {}
    void onPostChangeMidiEvent() override {}
    void onRemoveMidiEvent(const MidiEvent& event) override {}
    void onPostRemoveMidiEvent(MidiTrack* const layer) override {}

    void onAddTrack(MidiTrack* const track) override {}
    void onChangeTrackProperties(MidiTrack* const track) override {}
    void onRemoveTrack(MidiTrack* const track) override {}
    void onPostRemoveTrack() override {}

    void onAddPitchColorMapEntry(const PitchColorMapEntry& entry) override {}
    void onPostAddPitchColorMapEntry() override {}
    void onChangePitchColorMapEntry(const PitchColorMapEntry& oldEntry, const PitchColorMapEntry& newEntry) override {}
    void onPostChangePitchColorMapEntry() override {}
    void onRemovePitchColorMapEntry(const PitchColorMapEntry& entry) override {}
    void onPostRemovePitchColorMapEntry() override {}
    void onChangePitchColorMap(PitchColorMap* const colorMap) override {}

    void onPostTuningChange() override {}

    void onChangeProjectBeatRange(float firstBeat, float lastBeat) override {}
    void onChangeViewBeatRange(float firstBeat, float lastBeat) override {}

    void onReloadProjectContent(const Array<MidiTrack*>& tracks) override;

private:
    MicroChromoAudioProcessor& processor;
    ApplicationProperties& appProperties;
    AudioPluginFormatManager& formatManager;
    ApplicationCommandManager commandManager;
    UndoManager& undoManager;
    std::shared_ptr<PluginBundle> synthBundle, psBundle;

    //==============================================================================
    KnownPluginList& knownPluginList;
    KnownPluginList::SortMethod pluginSortMethod;

    //==============================================================================
    std::unique_ptr<MainEditor> mainEditor;
    std::unique_ptr<SimpleNoteEditor> noteEditor;
    std::unique_ptr<SimpleTimeEditor> timeEditor;
    std::unique_ptr<ColorEditor> colorEditor;
    std::unique_ptr<TuningEditor> tuningEditor;

    //==============================================================================
    std::unique_ptr<MenuBarComponent> menuBar;
    TextButton playTransportBtn{ "Play" }, stopTransportBtn{ "Stop" }, panicBtn{ "!" };
    Slider transportSlider;
    Label transportLabel;
    std::atomic<bool> transportSliderDragging{ false };

    //==============================================================================
    class CustomTabbedComponent : public TabbedComponent
    {
    public:
        CustomTabbedComponent(TabbedButtonBar::Orientation orientation, MicroChromoAudioProcessorEditor& owner);
        ~CustomTabbedComponent() = default;

        virtual void currentTabChanged(int newCurrentTabIndex, const String& /*newCurrentTabName*/) override;

    private:
        MicroChromoAudioProcessorEditor& _owner;
    };
    std::unique_ptr<CustomTabbedComponent> tabComp;

    //==============================================================================
    class PluginListWindow : public DocumentWindow
    {
    public:
        PluginListWindow(MicroChromoAudioProcessorEditor& mw, AudioPluginFormatManager& pluginFormatManager);
        ~PluginListWindow();

        void closeButtonPressed();
    private:
        MicroChromoAudioProcessorEditor& owner;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginListWindow)
    };
    std::unique_ptr<PluginListWindow> pluginListWindow;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MicroChromoAudioProcessorEditor)
};
