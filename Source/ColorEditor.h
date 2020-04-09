/*
  ==============================================================================

    ColorEditor.h
    Created: 8 Apr 2020 9:50:04am
    Author:  bowen

  ==============================================================================
*/

#pragma once

#include "Common.h"

class Note;
class SimpleMidiEditor;
class MicroChromoAudioProcessorEditor;

//==============================================================================
/*
*/
class ColorEditor : public ComponentWithTable
{
public:
    ColorEditor(MicroChromoAudioProcessorEditor& editor, SimpleMidiEditor& midiEditor);
    ~ColorEditor();

    int getNumRows() override;

    void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override;
    void cellDoubleClicked(int rowNumber, int columnId, const MouseEvent&) override;

    int getColumnAutoSizeWidth(int columnId) override;

    String getText(const int rowNumber, const int columnNumber) const override;
    void setText(const int rowNumber, const int columnNumber, const String& newText) override;

    //void buttonClicked(const int rowNumber, const int columnNumber, const Colour& newColor) override;
    void colorChooserClosed(int rowNumber, int columnId, const Colour& color);

    void updateColorMapList() override;

    void paint (Graphics&) override;
    void resized() override;

private:
    SimpleMidiEditor& _midiEditor;

    //==============================================================================
    class ColorChooserComponent : public Component
    {
    public:
        ColorChooserComponent(ColorEditor& parent, const int row, const int col, const Colour& color);
        ~ColorChooserComponent() = default;

        //==============================================================================
        void paint(Graphics& g) override;
        void resized() override;

    private:
        ColorEditor& _parent;

        ColourSelector selector;
        TextButton btn{ "Done" };

        int rowId{ -1 }, columnId{ -1 };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColorChooserComponent)
    };

    HashMap<String, ColorPitchBendRecord>& noteColorMap;
    Array<ColorPitchBendRecord> noteColorMapList;

    TextButton addBtn{ "Add" }, removeBtn{ "Remove" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColorEditor)
};
