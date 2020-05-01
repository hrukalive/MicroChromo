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
class MicroChromoAudioProcessorEditor;

class PluginBundle;

class MidiEvent;
class MidiTrack;
class PitchColorMapEntry;
class PitchColorMap;

//==============================================================================
class MainEditor :
    public AudioProcessorEditor,
    public ChangeListener,
    public ProjectListener,
    public Timer,
    public DragAndDropContainer
{
public:
    MainEditor(MicroChromoAudioProcessor& p, MicroChromoAudioProcessorEditor& parent);
    ~MainEditor();

    //===------------------------------------------------------------------===//
    // Components
    //===------------------------------------------------------------------===//
    void paint(Graphics&) override;
    void resized() override;

    //===------------------------------------------------------------------===//
    // Callbacks
    //===------------------------------------------------------------------===//
    void changeListenerCallback(ChangeBroadcaster*) override;
    void timerCallback() override;
    void itemDroppedCallback(const Array<File>& files);

    //===------------------------------------------------------------------===//
    // Mouse Listeners
    //===------------------------------------------------------------------===//
    void mouseDoubleClick(const MouseEvent& event) override;
    void mouseDrag(const MouseEvent&) override;

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
    //===------------------------------------------------------------------===//
    // Helpers
    //===------------------------------------------------------------------===//
    void exportMidiDialog();

    MicroChromoAudioProcessor& processor;
    MicroChromoAudioProcessorEditor& _parent;
    ApplicationProperties& appProperties;
    std::shared_ptr<PluginBundle> synthBundle, psBundle;

    //==============================================================================
    friend class MicroChromoAudioProcessorEditor;

    //==============================================================================
    class TextButtonDropTarget : public TextButton, public FileDragAndDropTarget
    {
    public:
        TextButtonDropTarget(String text, MainEditor& owner);
        ~TextButtonDropTarget() = default;

        bool isInterestedInFileDrag(const StringArray& files) override;
        void filesDropped(const StringArray& files, int x, int y) override;

    private:
        MainEditor& _owner;
    };

    //==============================================================================
    std::unique_ptr<Button> synthButton, effectButton, dragButton, dropButton;
    std::unique_ptr<Label> synthLabel, effectLabel;
    std::unique_ptr<Label> numInstancesLabel, numParameterLabel, midiChannelLabel, pbRangeLabel, tailLenLabel;
    std::unique_ptr<ComboBox> numInstancesBox, midiChannelComboBox;
    std::unique_ptr<TextEditor> numParameterSlot, pbRangeTextbox, tailLenTextbox;
    std::unique_ptr<PopupMenu> floatMenu;

    std::function<void(int, bool)> bundlePopupMenuSelected;

    //==============================================================================
    String lastOpenedLocation = "", lastSavedLocation = "";
    bool canDrop = true;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainEditor)
};