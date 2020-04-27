/*
  ==============================================================================

    SimpleNoteEditor.cpp
    Created: 16 Apr 2020 9:09:39pm
    Author:  bowen

  ==============================================================================
*/

#include "SimpleNoteEditor.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Note.h"
#include "MidiTrack.h"

//==============================================================================
SimpleNoteEditor::SimpleNoteEditor(MicroChromoAudioProcessorEditor& editor) :
    owner(editor), processor(editor.getProcessor()), project(editor.getProcessor().getProject())
{
    int columnId = 1;
    table.getHeader().addColumn("ID", columnId++, 30, 30, 50, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("Note#", columnId++, 50, 50, 100, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("", columnId++, 50, 50, 100, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("Start", columnId++, 50, 50, 100, TableHeaderComponent::notSortable | TableHeaderComponent::sortedForwards);
    table.getHeader().addColumn("Bar", columnId++, 50, 50, 100, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("Beat", columnId++, 50, 50, 100, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("Length", columnId++, 50, 50, 100, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("Velocity", columnId++, 50, 50, 100, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("Color", columnId++, 120, 100, 200, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("", columnId++, 30, 30, 30, TableHeaderComponent::notSortable);
    
    table.setColour(ListBox::outlineColourId, Colours::grey);
    table.setOutlineThickness(1);
    table.setMultipleSelectionEnabled(true);
    table.getHeader().setSortColumnId(4, true);

    addAndMakeVisible(table);

    addAndMakeVisible(trackCombo);
    trackCombo.onChange = [&]() {
        table.updateContent();
        table.repaint();
    };

    addAndMakeVisible(addTrackBtn);
    addTrackBtn.onClick = [&]() {
        processor.getUndoManager().beginNewTransaction("'Add a track'");
        project.addTrack(ValueTree(), "empty", 1, true);
    };
    addAndMakeVisible(removeTrackBtn);
    removeTrackBtn.onClick = [&]() {
        if (auto* track = project.findTrackById<NoteTrack>(trackCombo.getSelectedId()))
        {
            processor.getUndoManager().beginNewTransaction("'Remove track'");
            project.removeTrack(track->getTrackId(), true);
        }
    };
    addAndMakeVisible(changeChannelBtn);
    changeChannelBtn.onClick = [&]() {
        if (auto* track = project.findTrackById<NoteTrack>(trackCombo.getSelectedId()))
        {
            AlertWindow* window = new AlertWindow("Change Channel", "Select MIDI channel", AlertWindow::AlertIconType::NoIcon);
            window->addComboBox("channelComboBox", { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16" });
            window->getComboBoxComponent("channelComboBox")->setSelectedId(track->getTrackChannel());
            window->addButton(TRANS("OK"), 1, KeyPress(KeyPress::returnKey));
            window->addButton(TRANS("Cancel"), 0, KeyPress(KeyPress::escapeKey));

            window->enterModalState(true,
                ModalCallbackFunction::create([window, track, this](int r) {
                    if (r == 1 && window->getComboBoxComponent("channelComboBox")->getSelectedId() != track->getTrackChannel())
                    {
                        this->processor.getUndoManager().beginNewTransaction("'Change track channel'");
                        track->setTrackChannel(window->getComboBoxComponent("channelComboBox")->getSelectedId(), true, true);
                    }
                    }),
                true);
        }
    };
    addAndMakeVisible(renameBtn);
    renameBtn.onClick = [&]() {
        if (auto* track = project.findTrackById<NoteTrack>(trackCombo.getSelectedId()))
        {
            AlertWindow* window = new AlertWindow("Rename", "Enter new name", AlertWindow::AlertIconType::NoIcon);
            window->addTextEditor("nameEditor", track->getTrackName());
            window->addButton(TRANS("OK"), 1, KeyPress(KeyPress::returnKey));
            window->addButton(TRANS("Cancel"), 0, KeyPress(KeyPress::escapeKey));

            window->enterModalState(true,
                ModalCallbackFunction::create([window, track, this](int r) {
                    if (r == 1 && window->getTextEditorContents("nameEditor") != track->getTrackName())
                    {
                        this->processor.getUndoManager().beginNewTransaction("'Rename track'");
                        track->setTrackName(window->getTextEditorContents("nameEditor"), true, true);
                    }
                    }),
                true);
        }
    };

    addAndMakeVisible(addBtn);
    addBtn.onClick = [&]() {
        if (auto* track = project.findTrackById<NoteTrack>(trackCombo.getSelectedId()))
        {
            processor.getUndoManager().beginNewTransaction("'Add a note'");
            track->insert(Note(track, 60, jmax(0.0f, track->getLastBeat()), 1, 0.8, project.getPitchColorMap()->findDefaultColorForKey(60)), true);
        }
    };

    addAndMakeVisible(removeBtn);
    removeBtn.onClick = [&]() {
        if (auto* track = project.findTrackById<NoteTrack>(trackCombo.getSelectedId()))
        {
            Array<Note> selectedNotes;
            for (int row = 0; row < track->size(); row++)
                if (table.isRowSelected(row))
                    selectedNotes.add(*(*track)[row]);
            processor.getUndoManager().beginNewTransaction("'Remove selected note(s)'");
            track->removeGroup(selectedNotes, true);
        }
    };

    addAndMakeVisible(copyBtn);
    copyBtn.onClick = [&]() {
        copiedNotes = nullptr;
        if (auto* track = project.findTrackById<NoteTrack>(trackCombo.getSelectedId()))
        {
            Array<Note> selectedNotes;
            for (int row = 0; row < track->size(); row++)
                if (table.isRowSelected(row))
                    selectedNotes.add(*(*track)[row]);

            if (!selectedNotes.isEmpty())
                copiedNotes.reset(new std::pair<int, Array<Note>>(trackCombo.getSelectedId(), selectedNotes));
        }
    };

    addAndMakeVisible(pasteBtn);
    pasteBtn.onClick = [&]() {
        DBG("Try paste to track " << trackCombo.getSelectedId());
        if (auto* track = project.findTrackById<NoteTrack>(trackCombo.getSelectedId()))
        {
            if (copiedNotes != nullptr && track->getTrackId() != copiedNotes->first)
            {
                Array<Note> copied;
                for (auto& n : copiedNotes->second)
                {
                    Note newNote(n.getTrack());
                    newNote.deserialize(n.serialize());
                    copied.add(newNote);
                }
                processor.getUndoManager().beginNewTransaction("'Paste note(s)'");
                track->insertGroup(copied, true);
            }
        }
    };

    addAndMakeVisible(updateBtn);
    updateBtn.onClick = [&]() {
        processor.updateMidiSequence();
    };

    project.addListener(this);

    trackCombo.clear();
    for (auto& track : project)
        trackCombo.addItem(track->getTrackName(), track->getTrackId());
    if (project.size() > 0)
        trackCombo.setSelectedId((*project.begin())->getTrackId());

    setSize(500, 300);
}

SimpleNoteEditor::~SimpleNoteEditor()
{
    project.removeListener(this);
}

int SimpleNoteEditor::getNumRows()
{
    if (auto* track = project.findTrackById<NoteTrack>(trackCombo.getSelectedId()))
        return track->size();
    return 0;
}

void SimpleNoteEditor::paintRowBackground(Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected)
{
    auto alternateColour = getLookAndFeel().findColour(ListBox::backgroundColourId)
        .interpolatedWith(getLookAndFeel().findColour(ListBox::textColourId), 0.03f);
    if (rowIsSelected)
        g.fillAll(findColour(TextEditor::highlightColourId));
    else if (rowNumber % 2)
        g.fillAll(alternateColour);
}

void SimpleNoteEditor::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/)
{
    g.setColour(getLookAndFeel().findColour(ListBox::textColourId).darker());
    g.setFont(font);

    auto* track = project.findTrackById<NoteTrack>(trackCombo.getSelectedId());
    if (columnId == 1)
        g.drawText(String((*track)[rowNumber]->getId()), 2, 0, width - 4, height, Justification::centredLeft, true);
    else if (columnId == 3)
        g.drawText(MidiMessage::getMidiNoteName((*track)[rowNumber]->getKey(), true, true, 4), 2, 0, width - 4, height, Justification::centredLeft, true);
    else if (columnId == 5)
        g.drawText(String(project.getBarAndBeat((*track)[rowNumber]->getBeat()).first), 2, 0, width - 4, height, Justification::centredLeft, true);
    else if (columnId == 6)
        g.drawText(String(project.getBarAndBeat((*track)[rowNumber]->getBeat()).second, 2), 2, 0, width - 4, height, Justification::centredLeft, true);
    else if (columnId == 10)
    {
        auto* c = project.getPitchColorMap()->findEntryByName((*track)[rowNumber]->getPitchColor());
        if (c)
            g.setColour(c->getColor());
        else
            g.setColour(Colours::black);
        g.fillRect(4, 4, width - 8, height - 8);
    }

    g.setColour(getLookAndFeel().findColour(ListBox::backgroundColourId));
    g.fillRect(width - 1, 0, 1, height);
}

Component* SimpleNoteEditor::refreshComponentForCell(int rowNumber, int columnId, bool /*isRowSelected*/, Component* existingComponentToUpdate)
{
    if (columnId == 2 || columnId  == 4 || columnId == 7 || columnId == 8)
    {
        auto* textLabel = static_cast<EditableTextCustomComponent*> (existingComponentToUpdate);
        if (textLabel == nullptr)
            textLabel = new EditableTextCustomComponent(*this);
        textLabel->setRowAndColumn(rowNumber, columnId);
        return textLabel;
    }
    if (columnId == 9)
    {
        auto* comboBox = static_cast<ComboBoxCustomComponent*>(existingComponentToUpdate);
        if (comboBox == nullptr)
            comboBox = new ComboBoxCustomComponent(*this);
        comboBox->clear();
        for (auto* entry : *project.getPitchColorMap())
            comboBox->addItem(entry->getName(), entry->getEntryId());
        comboBox->setRowAndColumn(rowNumber, columnId);
        return comboBox;
    }

    jassert(existingComponentToUpdate == nullptr);
    return nullptr;
}

String SimpleNoteEditor::getText(const int rowNumber, const int columnNumber) const
{
    String text;
    auto* track = project.findTrackById<NoteTrack>(trackCombo.getSelectedId());
    switch (columnNumber)
    {
    case 2: text = String((*track)[rowNumber]->getKey()); break;
    case 4: text = String((*track)[rowNumber]->getBeat(), 2); break;
    case 7: text = String((*track)[rowNumber]->getLength(), 2); break;
    case 8: text = String((*track)[rowNumber]->getVelocity(), 2); break;
    default:
        break;
    }
    return text;
}

void SimpleNoteEditor::setText(const int rowNumber, const int columnNumber, const String& newText)
{
    auto* track = project.findTrackById<NoteTrack>(trackCombo.getSelectedId());
    auto* note = (*track)[rowNumber];
    switch (columnNumber)
    {
    case 2: {
        if (track && note->getKey() != newText.getIntValue())
        {
            processor.getUndoManager().beginNewTransaction("'Change note number'");
            track->change(*note, (*note).withKey(newText.getIntValue())
                .withPitchColor(project.getPitchColorMap()->findDefaultColorForKey(newText.getIntValue())), true);
        }
        break;
    }
    case 4: {
        if (track && note->getBeat() != roundBeat(newText.getFloatValue()))
        {
            processor.getUndoManager().beginNewTransaction("'Change note position'");
            track->change(*note, (*note).withBeat(newText.getFloatValue()), true);
        }
        break;
    }
    case 7: {
        if (track && note->getLength() != newText.getFloatValue())
        {
            processor.getUndoManager().beginNewTransaction("'Change note length'");
            track->change(*note, (*note).withLength(newText.getFloatValue()), true);
        }
        break;
    }
    case 8: {
        if (track && note->getVelocity() != newText.getFloatValue())
        {
            processor.getUndoManager().beginNewTransaction("'Change note velocity'");
            track->change(*note, (*note).withVelocity(newText.getFloatValue()), true);
        }
        break;
    }
    default:
        break;
    }
}

int SimpleNoteEditor::getSelectedId(const int rowNumber, const int columnNumber) const
{
    if (columnNumber == 9)
    {
        auto* track = project.findTrackById<NoteTrack>(trackCombo.getSelectedId());
        if (auto* c = project.getPitchColorMap()->findEntryByName((*track)[rowNumber]->getPitchColor()))
            return c->getEntryId();
    }
    return 0;
}

void SimpleNoteEditor::setSelectedId(const int rowNumber, const int /*columnNumber*/, const int newId)
{
    if (getSelectedId(rowNumber, 9) != newId)
    {
        auto* track = project.findTrackById<NoteTrack>(trackCombo.getSelectedId());

        if (!table.isRowSelected(rowNumber))
            table.selectRow(rowNumber);

        Array<Note> before, after;
        for (int row = 0; row < track->size(); row++)
        {
            if (table.isRowSelected(row))
            {
                before.add(*((*track)[row]));
                after.add((*track)[row]->withPitchColor(project.getPitchColorMap()->findEntryById(newId)->getName()));
            }
        }
        processor.getUndoManager().beginNewTransaction("'Change note color'");
        track->changeGroup(before, after, true);
    }
}

void SimpleNoteEditor::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void SimpleNoteEditor::resized()
{
    auto b = getLocalBounds();
    b.reduce(10, 10);
    auto fifth = b.proportionOfWidth(1.0 / 5);
    auto sixth = b.proportionOfWidth(1.0 / 6);

    auto tmp = b.removeFromTop(28);
    b.removeFromTop(4);
    addTrackBtn.setBounds(tmp.removeFromLeft(sixth).reduced(2));
    removeTrackBtn.setBounds(tmp.removeFromLeft(sixth).reduced(2));
    changeChannelBtn.setBounds(tmp.removeFromRight(sixth).reduced(2));
    renameBtn.setBounds(tmp.removeFromRight(sixth).reduced(2));
    trackCombo.setBounds(tmp.reduced(2));

    tmp = b.removeFromBottom(28);
    b.removeFromBottom(4);
    addBtn.setBounds(tmp.removeFromLeft(fifth).reduced(2));
    removeBtn.setBounds(tmp.removeFromLeft(fifth).reduced(2));
    copyBtn.setBounds(tmp.removeFromLeft(fifth).reduced(2));
    pasteBtn.setBounds(tmp.removeFromLeft(fifth).reduced(2));
    updateBtn.setBounds(tmp.removeFromLeft(fifth).reduced(2));

    table.setBounds(b);
}

//===------------------------------------------------------------------===//
// Project Listener
//===------------------------------------------------------------------===//
void SimpleNoteEditor::onAddMidiEvent(const MidiEvent& event) {}
void SimpleNoteEditor::onPostAddMidiEvent()
{
    table.updateContent();
    table.repaint();
}
void SimpleNoteEditor::onChangeMidiEvent(const MidiEvent& oldEvent, const MidiEvent& newEvent) {}
void SimpleNoteEditor::onPostChangeMidiEvent()
{
    table.updateContent();
    table.repaint();
}
void SimpleNoteEditor::onRemoveMidiEvent(const MidiEvent& event) {}
void SimpleNoteEditor::onPostRemoveMidiEvent(MidiTrack* const layer)
{
    table.updateContent();
    table.repaint();
}

void SimpleNoteEditor::onAddTrack(MidiTrack* const track)
{
    trackCombo.clear();
    for (auto& track : project)
        trackCombo.addItem(track->getTrackName(), track->getTrackId());
    trackCombo.setSelectedId(track->getTrackId());
}
void SimpleNoteEditor::onChangeTrackProperties(MidiTrack* const track)
{
    trackCombo.clear();
    for (auto& track : project)
        trackCombo.addItem(track->getTrackName(), track->getTrackId());
    trackCombo.setSelectedId(track->getTrackId());
}
void SimpleNoteEditor::onRemoveTrack(MidiTrack* const track) {}
void SimpleNoteEditor::onPostRemoveTrack()
{
    trackCombo.clear();
    for (auto& track : project)
        trackCombo.addItem(track->getTrackName(), track->getTrackId());
    if (project.size() > 0)
        trackCombo.setSelectedId((*project.begin())->getTrackId());
}

void SimpleNoteEditor::onAddPitchColorMapEntry(const PitchColorMapEntry& entry) {}
void SimpleNoteEditor::onPostAddPitchColorMapEntry()
{
    table.updateContent();
    table.repaint();
}
void SimpleNoteEditor::onChangePitchColorMapEntry(const PitchColorMapEntry& oldEntry, const PitchColorMapEntry& newEntry)
{
    auto* track = project.findTrackById<NoteTrack>(trackCombo.getSelectedId());
    if (track)
    {
        Array<Note> before, after;
        bool hasChanged = false;
        for (auto* evt : *track)
        {
            if (auto* note = dynamic_cast<Note*>(evt))
            {
                if (note->getPitchColor() == oldEntry.getName())
                {
                    before.add(*note);
                    after.add(note->withPitchColor(newEntry.getName()));
                    hasChanged = true;
                }
            }
        }
        if (hasChanged)
            track->changeGroup(before, after, false);
    }
    for (auto* tk : project)
    {
        if (tk == track)
            break;
        Array<Note> before, after;
        bool hasChanged = false;
        for (auto* evt : *tk)
        {
            if (auto* note = dynamic_cast<Note*>(evt))
            {
                if (note->getPitchColor() == oldEntry.getName())
                {
                    before.add(*note);
                    after.add(note->withPitchColor(newEntry.getName()));
                    hasChanged = true;
                }
            }
        }
        if (hasChanged)
            tk->changeGroup(before, after, false);
    }
}
void SimpleNoteEditor::onPostChangePitchColorMapEntry()
{
    table.updateContent();
    table.repaint();
}
void SimpleNoteEditor::onRemovePitchColorMapEntry(const PitchColorMapEntry& entry) {}
void SimpleNoteEditor::onPostRemovePitchColorMapEntry()
{
    auto* track = project.findTrackById<NoteTrack>(trackCombo.getSelectedId());
    if (track)
    {
        Array<Note> before, after;
        bool hasChanged = false;
        for (auto* evt : *track)
        {
            if (auto* note = dynamic_cast<Note*>(evt))
            {
                if (!project.getPitchColorMap()->hasNamedUsed(note->getPitchColor()))
                {
                    before.add(*note);
                    after.add(note->withPitchColor("0"));
                    hasChanged = true;
                }
            }
        }
        if (hasChanged)
            track->changeGroup(before, after, false);
    }
    for (auto* tk : project)
    {
        Array<Note> before, after;
        bool hasChanged = false;
        for (auto* evt : *tk)
        {
            if (auto* note = dynamic_cast<Note*>(evt))
            {
                if (!project.getPitchColorMap()->hasNamedUsed(note->getPitchColor()))
                {
                    before.add(*note);
                    after.add(note->withPitchColor("0"));
                    hasChanged = true;
                }
            }
        }
        if (hasChanged)
            tk->changeGroup(before, after, false);
    }
    table.updateContent();
    table.repaint();
}
void SimpleNoteEditor::onChangePitchColorMap(PitchColorMap* const colorMap) {}

void SimpleNoteEditor::onChangeProjectBeatRange(float firstBeat, float lastBeat) {}
void SimpleNoteEditor::onChangeViewBeatRange(float firstBeat, float lastBeat) {}

void SimpleNoteEditor::onReloadProjectContent(const Array<MidiTrack*>& tracks)
{
    trackCombo.clear();
    for (auto& track : project)
        trackCombo.addItem(track->getTrackName(), track->getTrackId());
    if (project.size() > 0)
        trackCombo.setSelectedId((*project.begin())->getTrackId());
}
