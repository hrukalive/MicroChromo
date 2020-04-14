/*
  ==============================================================================

    SimpleMidiEditor.cpp
    Created: 8 Apr 2020 9:49:52am
    Author:  bowen

  ==============================================================================
*/

#include "SimpleMidiEditor.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Note.h"
#include "MidiTrack.h"

//==============================================================================
SimpleMidiEditor::SimpleMidiEditor(MicroChromoAudioProcessorEditor& editor) : 
    owner(editor), processor(editor.getProcessor()), notes(processor.getNotes()), noteColorMap(processor.getNoteColorMap())
{
    int columnId = 1;
    table.getHeader().addColumn("ID", columnId++, 50, 20, 100, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("Note", columnId++, 50, 50, 60, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("Start", columnId++, 80, 50, 120, TableHeaderComponent::notSortable | TableHeaderComponent::sortedForwards);
    table.getHeader().addColumn("Length", columnId++, 80, 50, 120, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("Velocity", columnId++, 50, 50, 60, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("Color", columnId++, 120, 100, 200, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("C", columnId++, 30, 30, 30, TableHeaderComponent::notSortable);

    table.setColour(ListBox::outlineColourId, Colours::grey);
    table.setOutlineThickness(1);
    table.setMultipleSelectionEnabled(true);
    table.getHeader().setSortColumnId(3, true);

    addAndMakeVisible(table);

    addAndMakeVisible(addBtn);
    addBtn.onClick = [&]() {
        notes.add(Note(nullptr, MIDDLE_C, notes.getLast().getBeat()));
        table.updateContent();
    };

    addAndMakeVisible(removeBtn);
    removeBtn.onClick = [&]() {
        for (int row = notes.size() - 1; row >= 0; row--)
        {
            if (table.isRowSelected(row))
                notes.removeAndReturn(row);
        }
        table.deselectAllRows();
        table.updateContent();
    };

    addAndMakeVisible(updateBtn);
    updateBtn.onClick = [&]() {
        processor.updateMidiSequence();
    };

    updateColorMapList();

    setSize(500, 300);
}

int SimpleMidiEditor::getNumRows()
{
    return notes.size();
}

void SimpleMidiEditor::paintRowBackground(Graphics & g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected)
{
    auto alternateColour = getLookAndFeel().findColour(ListBox::backgroundColourId)
        .interpolatedWith(getLookAndFeel().findColour(ListBox::textColourId), 0.03f);
    if (rowIsSelected)
        g.fillAll(findColour(TextEditor::highlightColourId));
    else if (rowNumber % 2)
        g.fillAll(alternateColour);
}

void SimpleMidiEditor::paintCell(Graphics & g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/)
{
    g.setColour(getLookAndFeel().findColour(ListBox::textColourId));
    g.setFont(font);

    if (columnId == 1)
    {
        g.drawText(String(notes[rowNumber].getId()), 2, 0, width - 4, height, Justification::centredLeft, true);
    }
    else if (columnId == 7)
    {
        if (noteColorMap.contains(notes[rowNumber].getPitchColor()))
            g.setColour(noteColorMap[notes[rowNumber].getPitchColor()].color);
        else
            g.setColour(Colours::black);
        g.fillRect(4, 4, width - 8, height - 8);
    }

    g.setColour(getLookAndFeel().findColour(ListBox::backgroundColourId));
    g.fillRect(width - 1, 0, 1, height);
}

Component* SimpleMidiEditor::refreshComponentForCell(int rowNumber, int columnId, bool /*isRowSelected*/, Component * existingComponentToUpdate)
{
    if (columnId >= 2 && columnId < 6)
    {
        auto* textLabel = static_cast<EditableTextCustomComponent*> (existingComponentToUpdate);
        if (textLabel == nullptr)
            textLabel = new EditableTextCustomComponent(*this);
        textLabel->setRowAndColumn(rowNumber, columnId);
        return textLabel;
    }
    if (columnId == 6)
    {
        auto* comboBox = static_cast<ComboBoxCustomComponent*>(existingComponentToUpdate);
        if (comboBox == nullptr)
            comboBox = new ComboBoxCustomComponent(*this);
        comboBox->updateList();
        comboBox->setRowAndColumn(rowNumber, columnId);
        return comboBox;
    }

    jassert(existingComponentToUpdate == nullptr);
    return nullptr;
}

int SimpleMidiEditor::getColumnAutoSizeWidth(int columnId)
{
    int widest = 92;

    for (auto i = getNumRows(); --i >= 0;)
    {

        String text;
        switch (columnId)
        {
        case 1: text = String(notes[i].getId()); break;
        case 2: text = String(notes[i].getKey()); break;
        case 3: text = String(notes[i].getBeat(), 2); break;
        case 4: text = String(notes[i].getLength(), 2); break;
        case 5: text = String(notes[i].getVelocity(), 2); break;
        case 6: text = String(notes[i].getPitchColor()); break;
        default:
            break;
        }
        widest = jmax(widest, font.getStringWidth(text));
    }

    return widest + 8;
}
String SimpleMidiEditor::getText(const int rowNumber, const int columnNumber) const
{
    String text;
    switch (columnNumber)
    {
    case 2: text = String(notes[rowNumber].getKey()); break;
    case 3: text = String(notes[rowNumber].getBeat(), 2); break;
    case 4: text = String(notes[rowNumber].getLength(), 2); break;
    case 5: text = String(notes[rowNumber].getVelocity(), 2); break;
    default:
        break;
    }
    return text;
}

void SimpleMidiEditor::setText(const int rowNumber, const int columnNumber, const String & newText)
{
    switch (columnNumber)
    {
    case 2: notes.getReference(rowNumber).setKey(newText.getIntValue()); break;
    case 3: notes.getReference(rowNumber).setBeat(newText.getFloatValue()); processor.sortNotes(); table.updateContent(); break;
    case 4: notes.getReference(rowNumber).setLength(newText.getFloatValue()); break;
    case 5: notes.getReference(rowNumber).setVelocity(newText.getFloatValue()); break;
    default:
        break;
    }
}

int SimpleMidiEditor::getSelectedId(const int rowNumber, const int columnNumber) const
{
    if (columnNumber == 6)
        return itemTextToitemId[notes[rowNumber].getPitchColor()];
    return 0;
}

void SimpleMidiEditor::setSelectedId(const int rowNumber, const int /*columnNumber*/, const int newId)
{
    String colorName = itemIdToitemText[newId];

    if (getSelectedId(rowNumber, 6) != newId)
    {
        if (!table.isRowSelected(rowNumber))
            table.selectRow(rowNumber);

        for (int row = 0; row < notes.size(); row++)
        {
            if (table.isRowSelected(row))
            {
                notes.getReference(row).setPitchColor(colorName);
                dynamic_cast<ComboBoxCustomComponent*>(table.getCellComponent(6, row))->setSelectedId(newId, dontSendNotification);
                table.repaintRow(row);
            }
        }
    }
}

void SimpleMidiEditor::updateColorMapList()
{
    Array< ColorPitchBendRecord> noteColorMapList;
    HashMap<String, ColorPitchBendRecord>::Iterator i(noteColorMap);
    while (i.next())
        noteColorMapList.add(i.getValue());

    noteColorMapList.sort(ColorPitchBendRecordComparator());

    itemIdToitemText.clear();
    itemTextToitemId.clear();
    itemOrder.clear();

    int c = 1;
    for (auto& nc : noteColorMapList)
    {
        itemIdToitemText.set(c, nc.name);
        itemTextToitemId.set(nc.name, c);
        itemOrder.add(nc.name);
        c++;
    }

    table.updateContent();
}

void SimpleMidiEditor::updateTableContent()
{
    table.updateContent();
}

void SimpleMidiEditor::paint(Graphics & g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void SimpleMidiEditor::resized()
{
    auto b = getLocalBounds();
    b.reduce(10, 10);
    auto third = b.proportionOfWidth(0.3);
    auto space = b.proportionOfWidth(0.1) / 2;

    auto tmp = b.removeFromBottom(24);
    b.removeFromBottom(space);
    addBtn.setBounds(tmp.removeFromLeft(third));
    tmp.removeFromLeft(space);
    removeBtn.setBounds(tmp.removeFromLeft(third));
    tmp.removeFromLeft(space);
    updateBtn.setBounds(tmp.removeFromLeft(third));

    table.setBounds(b);
}
