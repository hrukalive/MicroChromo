/*
  ==============================================================================

    SimpleMidiEditor.h
    Created: 8 Apr 2020 9:49:52am
    Author:  bowen

  ==============================================================================
*/

#pragma once

#include "Common.h"

class MidiTrack;
class Note;
class MicroChromoAudioProcessorEditor;

//==============================================================================
/*
*/
class SimpleMidiEditor : public ComponentWithTable
{
public:
    SimpleMidiEditor(MicroChromoAudioProcessorEditor& editor);
    ~SimpleMidiEditor() = default;

    int getNumRows() override;

    void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override;

    int getColumnAutoSizeWidth(int columnId) override;

    String getText(const int rowNumber, const int columnNumber) const override;
    void setText(const int rowNumber, const int columnNumber, const String& newText) override;

    int getSelectedId(const int rowNumber, const int columnNumber) const override;
    void setSelectedId(const int rowNumber, const int columnNumber, const int newId) override;

    void updateColorMapList() override;
    void updateTableContent();

    void paint(Graphics& g) override;
    void resized() override;

private:
    MicroChromoAudioProcessorEditor& owner;
    MicroChromoAudioProcessor& processor;
    Array<Note>& notes;
    HashMap<String, ColorPitchBendRecord>& noteColorMap;

    TextButton addBtn{ "Add" }, removeBtn{ "Remove" }, updateBtn{ "Update" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleMidiEditor)
};
