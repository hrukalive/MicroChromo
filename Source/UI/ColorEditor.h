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
#include "Project.h"
#include "ProjectListener.h"

class Note;
class SimpleMidiEditor;
class MicroChromoAudioProcessorEditor;

//==============================================================================
class ColorEditor : public ComponentWithTable, public ProjectListener
{
public:
    ColorEditor(MicroChromoAudioProcessorEditor& editor);
    ~ColorEditor();

    //===------------------------------------------------------------------===//
    // TableListModel
    //===------------------------------------------------------------------===//
    int getNumRows() override;

    void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override;
    void cellClicked(int rowNumber, int columnId, const MouseEvent&) override;

    //===------------------------------------------------------------------===//
    // ComponentWithTable
    //===------------------------------------------------------------------===//
    String getText(const int rowNumber, const int columnNumber) const override;
    void setText(const int rowNumber, const int columnNumber, const String& newText) override;

    void colorChooserClosed(int rowNumber, int columnId, const Colour& color);
    void defaultKeyChooserClosed(int rowNumber, int columnId, const std::unordered_set<int>& defaultKeys);

    //===------------------------------------------------------------------===//
    // Components
    //===------------------------------------------------------------------===//
    void paint(Graphics& g) override;
    void resized() override;

    //===------------------------------------------------------------------===//
    // Project Listener
    //===------------------------------------------------------------------===//
    void onAddMidiEvent(const MidiEvent& event) override {}
    void onPostAddMidiEvent() override {}
    void onChangeMidiEvent(const MidiEvent& oldEvent, const MidiEvent& newEvent) override {}
    void onPostChangeMidiEvent() override {}
    void onRemoveMidiEvent(const MidiEvent& event) override {}
    void onPostRemoveMidiEvent(MidiTrack* const layer) {}

    void onAddTrack(MidiTrack* const track) override {}
    void onChangeTrackProperties(MidiTrack* const track) override {}
    void onRemoveTrack(MidiTrack* const track) override {}
    void onPostRemoveTrack() override {}

    void onAddPitchColorMapEntry(const PitchColorMapEntry& entry) override {}
    void onPostAddPitchColorMapEntry() override;
    void onChangePitchColorMapEntry(const PitchColorMapEntry& oldEntry, const PitchColorMapEntry& newEntry) override {}
    void onPostChangePitchColorMapEntry() override;
    void onRemovePitchColorMapEntry(const PitchColorMapEntry& entry) override {}
    void onPostRemovePitchColorMapEntry() override;
    void onChangePitchColorMap(PitchColorMap* const colorMap) override;

    void onChangeProjectBeatRange(float firstBeat, float lastBeat) override {}
    void onChangeViewBeatRange(float firstBeat, float lastBeat) override {}

    void onReloadProjectContent(const Array<MidiTrack*>& tracks) override;


private:
    //===------------------------------------------------------------------===//
    // Helpers
    //===------------------------------------------------------------------===//
    void updatePresetComboBox(String name);
    PitchColorMap* findColorMapByName(String name);
    void checkModified();

    //===------------------------------------------------------------------===//
    // Presets
    //===------------------------------------------------------------------===//
    void loadColorPreset();
    void saveColorMapPresets();

    //==============================================================================
    MicroChromoAudioProcessorEditor& owner;
    MicroChromoAudioProcessor& processor;
    ApplicationProperties& appProperties;
    Project& project;

    //==============================================================================
    OwnedArray<PitchColorMap> colorMapPresets;
    bool hasPresetModified = false;

    //==============================================================================
    class ColorChooserComponent : public Component
    {
    public:
        ColorChooserComponent(ColorEditor& parent, const int row, const int col, const Colour& color);
        ~ColorChooserComponent() = default;

        //===------------------------------------------------------------------===//
        // Components
        //===------------------------------------------------------------------===//
        void paint(Graphics& g) override;
        void resized() override;

    private:
        ColorEditor& _parent;

        ColourSelector selector;
        TextButton btn{ "Done" };

        int rowId{ -1 }, columnId{ -1 };

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColorChooserComponent)
    };

    //==============================================================================
    class DefaultKeyChooser : public Component, public ListBoxModel
    {
    public:
        //==============================================================================
        DefaultKeyChooser(ColorEditor& parent, const int row, const int col, 
            const std::unordered_set<int>& used, const std::unordered_set<int>& current);
        ~DefaultKeyChooser() = default;

        //===------------------------------------------------------------------===//
        // Components
        //===------------------------------------------------------------------===//
        void paint(Graphics& g) override;
        void resized() override;

        //===------------------------------------------------------------------===//
        // ListBoxModel
        //===------------------------------------------------------------------===//
        int getNumRows() override { return 12; }
        void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;
        void listBoxItemClicked(int row, const MouseEvent& e) override;

    private:
        ColorEditor& _parent;
        int rowId{ -1 }, columnId{ -1 };

        std::unordered_set<int> usedSet;
        std::unordered_set<int> currentSet;
        std::unordered_set<int> selectedSet;

        ListBox noteList;
        TextButton okBtn{ "OK" };

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DefaultKeyChooser)
    };

    //==============================================================================
    TextButton addEntryBtn{ "+Entry" }, removeEntryBtn{ "-Entry" };
    TextButton savePresetBtn{ "Save" }, deletePresetBtn{ "Del" }, renamePresetBtn{ "Rename" };
    ComboBox presetComboBox;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColorEditor)
};
