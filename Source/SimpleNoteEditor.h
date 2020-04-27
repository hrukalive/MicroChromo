/*
  ==============================================================================

    SimpleNoteEditor.h
    Created: 16 Apr 2020 9:09:39pm
    Author:  bowen

  ==============================================================================
*/

#pragma once

#include "Common.h"
#include "Project.h"
#include "ProjectListener.h"

class MicroChromoAudioProcessorEditor;
class NoteTrack;

class SimpleNoteEditor : public ComponentWithTable, public ProjectListener
{
public:
    SimpleNoteEditor(MicroChromoAudioProcessorEditor& editor);
    ~SimpleNoteEditor();

    void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override;

    //===------------------------------------------------------------------===//
    // Accessors
    //===------------------------------------------------------------------===//
    int getNumRows() override;

    String getText(const int rowNumber, const int columnNumber) const override;
    void setText(const int rowNumber, const int columnNumber, const String& newText) override;

    int getSelectedId(const int rowNumber, const int columnNumber) const override;
    void setSelectedId(const int rowNumber, const int columnNumber, const int newId) override;

    //===------------------------------------------------------------------===//
    // Components
    //===------------------------------------------------------------------===//
    void paint(Graphics& g) override;
    void resized() override;

    //===------------------------------------------------------------------===//
    // Project Listener
    //===------------------------------------------------------------------===//
    void onAddMidiEvent(const MidiEvent& event) override;
    void onPostAddMidiEvent() override;
    void onChangeMidiEvent(const MidiEvent& oldEvent, const MidiEvent& newEvent) override;
    void onPostChangeMidiEvent() override;
    void onRemoveMidiEvent(const MidiEvent& event) override;
    void onPostRemoveMidiEvent(MidiTrack* const layer) override;

    void onAddTrack(MidiTrack* const track) override;
    void onChangeTrackProperties(MidiTrack* const track) override;
    void onRemoveTrack(MidiTrack* const track) override;
    void onPostRemoveTrack() override;

    void onAddPitchColorMapEntry(const PitchColorMapEntry& entry) override;
    void onPostAddPitchColorMapEntry() override;
    void onChangePitchColorMapEntry(const PitchColorMapEntry& oldEntry, const PitchColorMapEntry& newEntry) override;
    void onPostChangePitchColorMapEntry() override;
    void onRemovePitchColorMapEntry(const PitchColorMapEntry& entry) override;
    void onPostRemovePitchColorMapEntry() override;
    void onChangePitchColorMap(PitchColorMap* const colorMap) override;

    void onChangeProjectBeatRange(float firstBeat, float lastBeat) override;
    void onChangeViewBeatRange(float firstBeat, float lastBeat) override;

    void onReloadProjectContent(const Array<MidiTrack*>& tracks) override;

private:
    MicroChromoAudioProcessorEditor& owner;
    MicroChromoAudioProcessor& processor;
    Project& project;

    std::unique_ptr<std::pair<int, Array<Note>>> copiedNotes;

    ComboBox trackCombo;
    TextButton addBtn{ "+Note" }, removeBtn{ "-Note" }, copyBtn{ "Copy" }, pasteBtn{ "Paste" }, updateBtn{ "Update Seq" };
    TextButton addTrackBtn{ "+Track" }, removeTrackBtn{ "-Track" }, changeChannelBtn{ "Channel" }, renameBtn{ "Rename" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleNoteEditor)
};

