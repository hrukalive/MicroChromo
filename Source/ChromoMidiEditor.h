/*
  ==============================================================================

    ChromoMidiEditor.h
    Created: 4 Apr 2020 12:01:27pm
    Author:  bowen

  ==============================================================================
*/

#pragma once

#include "Common.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"

class ChromoMidiEditor : public Component, public TableListBoxModel
{
public:
    ChromoMidiEditor(MicroChromoAudioProcessorEditor& editor) : owner(editor), processor(editor.getProcessor())
    {
        table.getHeader().addColumn("Note", 1, 100, 100, 200, TableHeaderComponent::defaultFlags);
        table.getHeader().addColumn("Start", 2, 100, 100, 200, TableHeaderComponent::defaultFlags);
        table.getHeader().addColumn("End", 3, 100, 100, 200, TableHeaderComponent::defaultFlags);
        table.getHeader().addColumn("Color", 4, 100, 100, 200, TableHeaderComponent::defaultFlags);

        notes.add(new NoteRow(60, 2, 4, 0));
        notes.add(new NoteRow(63, 2, 4, -2));
        notes.add(new NoteRow(67, 2, 4, 0));
        notes.add(new NoteRow(60 + 12, 2, 4, 0));
        notes.add(new NoteRow(63 + 12, 2, 4, -2));
        notes.add(new NoteRow(67 + 12, 2, 4, 0));

        notes.add(new NoteRow(60, 5, 7, 0));
        notes.add(new NoteRow(63, 5, 7, 5));
        notes.add(new NoteRow(67, 5, 7, 0));

        notes.add(new NoteRow(60, 8, 11, 0));
        notes.add(new NoteRow(64, 8, 11, 3));
        notes.add(new NoteRow(67, 8, 11, 0));

        //notes.add(new NoteRow(60, 2, 10, -5));
        //notes.add(new NoteRow(61, 3, 10, -4));
        //notes.add(new NoteRow(62, 4, 10, -3));
        //notes.add(new NoteRow(63, 5, 10, -2));
        //notes.add(new NoteRow(64, 6, 10, -1));
        //notes.add(new NoteRow(65, 7, 10, 0));
        //notes.add(new NoteRow(66, 8, 10, 1));
        //notes.add(new NoteRow(67, 9, 10, 2));

        addAndMakeVisible(table);
        table.setColour(ListBox::outlineColourId, Colours::grey);
        table.setOutlineThickness(1);

        table.getHeader().setSortColumnId(1, true);
        table.setMultipleSelectionEnabled(false);

        addAndMakeVisible(updateMidBtn);
        updateMidBtn.onClick = [&]() {
            updateMidi();
        };

        setSize(500, 300);
    }

    ~ChromoMidiEditor()
    {
        notes.clear();
    }

    void updateMidi()
    {
        processor.clearNotes();
        for (auto& note : notes)
        {
            auto pitchbend = note->color / 10.0 * 100;
            if (pitchbend > 50)
                processor.addNote(Note(note->notenum + 1, note->start, note->end - note->start, 0.7f, pitchbend - 100));
            else if (pitchbend < -50)
                processor.addNote(Note(note->notenum - 1, note->start, note->end - note->start, 0.7f, pitchbend + 100));
            else
                processor.addNote(Note(note->notenum, note->start, note->end - note->start, 0.7f, pitchbend));
        }
        AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::InfoIcon, "Done", "Done");
        processor.updateMidiSequence();
    }

    int getNumRows() override
    {
        return notes.size();
    }

    void paintRowBackground(Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override
    {
        auto alternateColour = getLookAndFeel().findColour(ListBox::backgroundColourId)
            .interpolatedWith(getLookAndFeel().findColour(ListBox::textColourId), 0.03f);
        if (rowIsSelected)
            g.fillAll(Colours::lightblue);
        else if (rowNumber % 2)
            g.fillAll(alternateColour);
    }

    void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override
    {
        g.setColour(rowIsSelected ? Colours::darkblue : getLookAndFeel().findColour(ListBox::textColourId)); // [5]
        g.setFont(font);

        String text;
        switch (columnId)
        {
        case 1: text = String(notes[rowNumber]->notenum); break;
        case 2: text = String(notes[rowNumber]->start, 2); break;
        case 3: text = String(notes[rowNumber]->end, 2); break;
        case 4: text = String(notes[rowNumber]->color); break;
        default:
            break;
        }
        g.drawText(text, 2, 0, width - 4, height, Justification::centredLeft, true);


        g.setColour(getLookAndFeel().findColour(ListBox::backgroundColourId));
        g.fillRect(width - 1, 0, 1, height);                                             // [7]
    }

    Component* refreshComponentForCell(int rowNumber, int columnId, bool /*isRowSelected*/, Component* existingComponentToUpdate) override
    {
        auto* textLabel = static_cast<EditableTextCustomComponent*> (existingComponentToUpdate);

        if (textLabel == nullptr)
            textLabel = new EditableTextCustomComponent(*this);

        textLabel->setRowAndColumn(rowNumber, columnId);
        return textLabel;
    }

    int getColumnAutoSizeWidth(int columnId) override
    {
        if (columnId == 9)
            return 50;

        int widest = 32;

        for (auto i = getNumRows(); --i >= 0;)
        {

            String text;
            switch (columnId)
            {
            case 1: text = String(notes[i]->notenum); break;
            case 2: text = String(notes[i]->start, 2); break;
            case 3: text = String(notes[i]->end, 2); break;
            case 4: text = String(notes[i]->color); break;
            default:
                break;
            }
            widest = jmax(widest, font.getStringWidth(text));
        }

        return widest + 8;
    }
    String getText(const int columnNumber, const int rowNumber) const
    {
        String text;
        switch (columnNumber)
        {
        case 1: text = String(notes[rowNumber]->notenum); break;
        case 2: text = String(notes[rowNumber]->start, 2); break;
        case 3: text = String(notes[rowNumber]->end, 2); break;
        case 4: text = String(notes[rowNumber]->color); break;
        default:
            break;
        }
        return text;
    }
    void setText(const int columnNumber, const int rowNumber, const String& newText)
    {
        switch (columnNumber)
        {
        case 1: notes[rowNumber]->notenum = newText.getIntValue(); break;
        case 2: notes[rowNumber]->start = newText.getFloatValue(); break;
        case 3: notes[rowNumber]->end = newText.getFloatValue(); break;
        case 4: notes[rowNumber]->color = newText.getIntValue(); break;
        default:
            break;
        }
    }

    void paint(Graphics& g)
    {
        g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
    }

    void resized()
    {
        auto b = getLocalBounds();
        b.reduce(10, 10);
        updateMidBtn.setBounds(b.removeFromBottom(24));
        b.removeFromBottom(6);
        table.setBounds(b);
    }

private:
    class EditableTextCustomComponent : public Label
    {
    public:
        EditableTextCustomComponent(ChromoMidiEditor& td)
            : owner(td)
        {
            setEditable(false, true, false);
        }
        void mouseDown(const MouseEvent& event) override
        {
            owner.table.selectRowsBasedOnModifierKeys(row, event.mods, false);

            Label::mouseDown(event);
        }
        void textWasEdited() override
        {
            owner.setText(columnId, row, getText());
        }
        void setRowAndColumn(const int newRow, const int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            setText(owner.getText(columnId, row), dontSendNotification);
        }
    private:
        ChromoMidiEditor& owner;
        int row, columnId;
        Colour textColour;
    };

    struct NoteRow
    {
        NoteRow(int n, float s, float e, int c) : notenum(n), start(s), end(e), color(c) {}
        int notenum, color;
        float start, end;
    };
    MicroChromoAudioProcessorEditor& owner;
    MicroChromoAudioProcessor& processor;
    TableListBox table{ {}, this };
    TextButton updateMidBtn{ "Update" };
    Font font{ 14.0f };
    OwnedArray<NoteRow> notes;
};
