/*
  ==============================================================================

    SimpleTimeEditor.h
    Created: 16 Apr 2020 9:09:55pm
    Author:  bowen

  ==============================================================================
*/

#pragma once

#include "Common.h"
#include "Project.h"
#include "ProjectListener.h"

class MicroChromoAudioProcessorEditor;
class TempoTrack;
class TimeSignatureTrack;

class SimpleTempoTrackEditor : public ComponentWithTable, public ProjectListener
{
public:
    SimpleTempoTrackEditor(MicroChromoAudioProcessorEditor& editor);
    ~SimpleTempoTrackEditor();

    void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override;

    //===------------------------------------------------------------------===//
    // Accessors
    //===------------------------------------------------------------------===//
    int getNumRows() override;

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

    void onChangeProjectBeatRange(float firstBeat, float lastBeat) override {}
    void onChangeViewBeatRange(float firstBeat, float lastBeat) override {}

    void onReloadProjectContent(const Array<MidiTrack*>& tracks) override;

private:
    MicroChromoAudioProcessorEditor& owner;
    MicroChromoAudioProcessor& processor;
    Project& project;

    ComboBox trackCombo;
    TextButton addBtn{ "+Marker" }, removeBtn{ "-Marker" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleTempoTrackEditor)
};

class SimpleTimeSignatureTrackEditor : public ComponentWithTable, public ProjectListener
{
public:
    SimpleTimeSignatureTrackEditor(MicroChromoAudioProcessorEditor& editor);
    ~SimpleTimeSignatureTrackEditor();

    void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override;

    //===------------------------------------------------------------------===//
    // Accessors
    //===------------------------------------------------------------------===//
    int getNumRows() override;

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

    void onChangeProjectBeatRange(float firstBeat, float lastBeat) override {}
    void onChangeViewBeatRange(float firstBeat, float lastBeat) override {}

    void onReloadProjectContent(const Array<MidiTrack*>& tracks) override;

private:
    MicroChromoAudioProcessorEditor& owner;
    MicroChromoAudioProcessor& processor;
    Project& project;

    ComboBox trackCombo;
    TextButton addBtn{ "+Time Sig." }, removeBtn{ "-Time Sig." };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleTimeSignatureTrackEditor)
};

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleTimeEditor)
};
