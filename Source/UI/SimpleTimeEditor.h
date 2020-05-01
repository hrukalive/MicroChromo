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
class Project;
class TempoTrack;
class TimeSignatureTrack;

//==============================================================================
class SimpleTempoTrackEditor : public ComponentWithTable, public ProjectListener
{
public:
    SimpleTempoTrackEditor(MicroChromoAudioProcessorEditor& editor);
    ~SimpleTempoTrackEditor();

    //===------------------------------------------------------------------===//
    // TableListModel
    //===------------------------------------------------------------------===//
    int getNumRows() override;

    void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override;

    //===------------------------------------------------------------------===//
    // ComponentWithTable
    //===------------------------------------------------------------------===//
    String getText(const int rowNumber, const int columnNumber) const override;
    void setText(const int rowNumber, const int columnNumber, const String& newText) override;

    //===------------------------------------------------------------------===//
    // Components
    //===------------------------------------------------------------------===//
    void paint(Graphics& g) override;
    void resized() override;

    //===------------------------------------------------------------------===//
    // Project Listener
    //===------------------------------------------------------------------===//
    void onAddMidiEvent(const MidiEvent& event) override {}
    void onPostAddMidiEvent() override;
    void onChangeMidiEvent(const MidiEvent& oldEvent, const MidiEvent& newEvent) override {}
    void onPostChangeMidiEvent() override;
    void onRemoveMidiEvent(const MidiEvent& event) override {}
    void onPostRemoveMidiEvent(MidiTrack* const layer) override;

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
    MicroChromoAudioProcessorEditor& owner;
    MicroChromoAudioProcessor& processor;
    Project& project;

    //==============================================================================
    ComboBox trackCombo;
    TextButton addBtn{ "+Marker" }, removeBtn{ "-Marker" };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleTempoTrackEditor)
};

//==============================================================================
class SimpleTimeSignatureTrackEditor : public ComponentWithTable, public ProjectListener
{
public:
    SimpleTimeSignatureTrackEditor(MicroChromoAudioProcessorEditor& editor);
    ~SimpleTimeSignatureTrackEditor();

    //===------------------------------------------------------------------===//
    // TableListModel
    //===------------------------------------------------------------------===//
    int getNumRows() override;

    void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override;

    //===------------------------------------------------------------------===//
    // ComponentWithTable
    //===------------------------------------------------------------------===//
    String getText(const int rowNumber, const int columnNumber) const override;
    void setText(const int rowNumber, const int columnNumber, const String& newText) override;

    //===------------------------------------------------------------------===//
    // Components
    //===------------------------------------------------------------------===//
    void paint(Graphics& g) override;
    void resized() override;

    //===------------------------------------------------------------------===//
    // Project Listener
    //===------------------------------------------------------------------===//
    void onAddMidiEvent(const MidiEvent& event) override {}
    void onPostAddMidiEvent() override;
    void onChangeMidiEvent(const MidiEvent& oldEvent, const MidiEvent& newEvent) override {}
    void onPostChangeMidiEvent() override;
    void onRemoveMidiEvent(const MidiEvent& event) override {}
    void onPostRemoveMidiEvent(MidiTrack* const layer) override;

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
    MicroChromoAudioProcessorEditor& owner;
    MicroChromoAudioProcessor& processor;
    Project& project;

    //==============================================================================
    ComboBox trackCombo;
    TextButton addBtn{ "+Time Sig." }, removeBtn{ "-Time Sig." };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleTimeSignatureTrackEditor)
};

//==============================================================================
class SimpleTimeEditor : public Component
{
public:
    SimpleTimeEditor(MicroChromoAudioProcessorEditor& editor);
    ~SimpleTimeEditor() = default;

    //===------------------------------------------------------------------===//
    // Components
    //===------------------------------------------------------------------===//
    void paint(Graphics & g) override;
    void resized() override;

private:
    MicroChromoAudioProcessorEditor& owner;
    SimpleTempoTrackEditor tempoEditor{ owner };
    SimpleTimeSignatureTrackEditor timeSigEditor{ owner };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleTimeEditor)
};
