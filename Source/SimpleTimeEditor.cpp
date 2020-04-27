/*
  ==============================================================================

    SimpleTimeEditor.cpp
    Created: 16 Apr 2020 9:09:55pm
    Author:  bowen

  ==============================================================================
*/

#include "SimpleTimeEditor.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "TempoMarkerEvent.h"
#include "TimeSignatureEvent.h"
#include "TempoTrack.h"
#include "TimeSignatureTrack.h"

//==============================================================================
SimpleTempoTrackEditor::SimpleTempoTrackEditor(MicroChromoAudioProcessorEditor& editor) :
    owner(editor), processor(editor.getProcessor()), project(editor.getProcessor().getProject())
{
    int columnId = 1;
    table.getHeader().addColumn("ID", columnId++, 30, 30, 50, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("Start", columnId++, 50, 50, 100, TableHeaderComponent::notSortable | TableHeaderComponent::sortedForwards);
    table.getHeader().addColumn("Bar", columnId++, 50, 50, 100, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("Beat", columnId++, 50, 50, 100, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("BPM", columnId++, 50, 50, 100, TableHeaderComponent::notSortable);

    table.setColour(ListBox::outlineColourId, Colours::grey);
    table.setOutlineThickness(1);
    table.setMultipleSelectionEnabled(true);
    table.getHeader().setSortColumnId(2, true);

    addAndMakeVisible(table);

    addAndMakeVisible(addBtn);
    addBtn.onClick = [&]() {
        auto* track = project.getTempoTrack();
        processor.getUndoManager().beginNewTransaction("'Add a tempo marker'");
        track->insert(TempoMarkerEvent(track, jmax(0.0f, track->getLastBeat())), true);
    };

    addAndMakeVisible(removeBtn);
    removeBtn.onClick = [&]() {
        auto* track = project.getTempoTrack();
        Array<TempoMarkerEvent> selectedMarkers;
        for (int row = 0; row < track->size(); row++)
            if (table.isRowSelected(row))
                selectedMarkers.add(*(*track)[row]);
        processor.getUndoManager().beginNewTransaction("'Remove selected marker(s)'");
        track->removeGroup(selectedMarkers, true);
    };

    project.addListener(this);

    setSize(255, 300);
}

SimpleTempoTrackEditor::~SimpleTempoTrackEditor()
{
    project.removeListener(this);
}

int SimpleTempoTrackEditor::getNumRows()
{
    return project.getTempoTrack()->size();
}

void SimpleTempoTrackEditor::paintRowBackground(Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected)
{
    auto alternateColour = getLookAndFeel().findColour(ListBox::backgroundColourId)
        .interpolatedWith(getLookAndFeel().findColour(ListBox::textColourId), 0.03f);
    if (rowIsSelected)
        g.fillAll(findColour(TextEditor::highlightColourId));
    else if (rowNumber % 2)
        g.fillAll(alternateColour);
}

void SimpleTempoTrackEditor::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/)
{
    g.setColour(getLookAndFeel().findColour(ListBox::textColourId).darker());
    g.setFont(font);

    auto* track = project.getTempoTrack();
    if (columnId == 1)
        g.drawText(String((*track)[rowNumber]->getId()), 2, 0, width - 4, height, Justification::centredLeft, true);
    else if (columnId == 3)
        g.drawText(String(project.getBarAndBeat((*track)[rowNumber]->getBeat()).first), 2, 0, width - 4, height, Justification::centredLeft, true);
    else if (columnId == 4)
        g.drawText(String(project.getBarAndBeat((*track)[rowNumber]->getBeat()).second, 2), 2, 0, width - 4, height, Justification::centredLeft, true);

    g.setColour(getLookAndFeel().findColour(ListBox::backgroundColourId));
    g.fillRect(width - 1, 0, 1, height);
}

Component* SimpleTempoTrackEditor::refreshComponentForCell(int rowNumber, int columnId, bool /*isRowSelected*/, Component* existingComponentToUpdate)
{
    if (columnId == 2 || columnId == 5)
    {
        auto* textLabel = static_cast<EditableTextCustomComponent*> (existingComponentToUpdate);
        if (textLabel == nullptr)
            textLabel = new EditableTextCustomComponent(*this);
        textLabel->setRowAndColumn(rowNumber, columnId);
        return textLabel;
    }

    jassert(existingComponentToUpdate == nullptr);
    return nullptr;
}

String SimpleTempoTrackEditor::getText(const int rowNumber, const int columnNumber) const
{
    auto* track = project.getTempoTrack();
    String text;
    switch (columnNumber)
    {
    case 2: text = String((*track)[rowNumber]->getBeat(), 2); break;
    case 5: text = String((*track)[rowNumber]->getBPM()); break;
    default:
        break;
    }
    return text;
}

void SimpleTempoTrackEditor::setText(const int rowNumber, const int columnNumber, const String& newText)
{
    auto* track = project.getTempoTrack();
    auto* marker = (*track)[rowNumber];
    switch (columnNumber)
    {
    case 2: {
        if (track && marker->getBeat() != roundBeat(newText.getFloatValue()))
        {
            processor.getUndoManager().beginNewTransaction("'Change marker position'");
            track->change(*marker, (*marker).withBeat(newText.getFloatValue()), true);
        }
        break;
    }
    case 5: {
        if (track && marker->getBPM() != newText.getIntValue())
        {
            processor.getUndoManager().beginNewTransaction("'Change marker BPM'");
            track->change(*marker, (*marker).withBPM(newText.getIntValue()), true);
        }
        break;
    }
    default:
        break;
    }
}

void SimpleTempoTrackEditor::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void SimpleTempoTrackEditor::resized()
{
    auto b = getLocalBounds();
    b.reduce(10, 10);
    auto half = b.proportionOfWidth(1.0 / 2);

    auto tmp = b.removeFromBottom(28);
    b.removeFromBottom(4);
    addBtn.setBounds(tmp.removeFromLeft(half).reduced(2));
    removeBtn.setBounds(tmp.removeFromLeft(half).reduced(2));

    table.setBounds(b);
}

//===------------------------------------------------------------------===//
// Project Listener
//===------------------------------------------------------------------===//
void SimpleTempoTrackEditor::onPostAddMidiEvent()
{
    table.updateContent();
    table.repaint();
}

void SimpleTempoTrackEditor::onPostChangeMidiEvent()
{
    table.updateContent();
    table.repaint();
}

void SimpleTempoTrackEditor::onPostRemoveMidiEvent(MidiTrack* const layer)
{
    table.updateContent();
    table.repaint();
}

void SimpleTempoTrackEditor::onReloadProjectContent(const Array<MidiTrack*>& tracks)
{
    table.updateContent();
    table.repaint();
}


//==============================================================================
SimpleTimeSignatureTrackEditor::SimpleTimeSignatureTrackEditor(MicroChromoAudioProcessorEditor& editor) :
    owner(editor), processor(editor.getProcessor()), project(editor.getProcessor().getProject())
{
    int columnId = 1;
    table.getHeader().addColumn("ID", columnId++, 30, 30, 50, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("Start", columnId++, 50, 50, 100, TableHeaderComponent::notSortable | TableHeaderComponent::sortedForwards);
    table.getHeader().addColumn("Bar", columnId++, 50, 50, 100, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("Beat", columnId++, 50, 50, 100, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("Numerator", columnId++, 50, 50, 100, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("Denominator", columnId++, 50, 50, 100, TableHeaderComponent::notSortable);

    table.setColour(ListBox::outlineColourId, Colours::grey);
    table.setOutlineThickness(1);
    table.setMultipleSelectionEnabled(true);
    table.getHeader().setSortColumnId(2, true);

    addAndMakeVisible(table);

    addAndMakeVisible(addBtn);
    addBtn.onClick = [&]() {
        auto* track = project.getTimeSignatureTrack();
        processor.getUndoManager().beginNewTransaction("'Add a time signature'");
        track->insert(TimeSignatureEvent(track, jmax(0.0f, track->getLastBeat())), true);
    };

    addAndMakeVisible(removeBtn);
    removeBtn.onClick = [&]() {
        auto* track = project.getTimeSignatureTrack();
        Array<TimeSignatureEvent> selectedSigs;
        for (int row = 0; row < track->size(); row++)
            if (table.isRowSelected(row))
                selectedSigs.add(*(*track)[row]);
        processor.getUndoManager().beginNewTransaction("'Remove selected time signature(s)'");
        track->removeGroup(selectedSigs, true);
    };

    project.addListener(this);

    setSize(305, 300);
}

SimpleTimeSignatureTrackEditor::~SimpleTimeSignatureTrackEditor()
{
    project.removeListener(this);
}

int SimpleTimeSignatureTrackEditor::getNumRows()
{
    return project.getTimeSignatureTrack()->size();
}

void SimpleTimeSignatureTrackEditor::paintRowBackground(Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected)
{
    auto alternateColour = getLookAndFeel().findColour(ListBox::backgroundColourId)
        .interpolatedWith(getLookAndFeel().findColour(ListBox::textColourId), 0.03f);
    if (rowIsSelected)
        g.fillAll(findColour(TextEditor::highlightColourId));
    else if (rowNumber % 2)
        g.fillAll(alternateColour);
}

void SimpleTimeSignatureTrackEditor::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/)
{
    g.setColour(getLookAndFeel().findColour(ListBox::textColourId).darker());
    g.setFont(font);

    auto* track = project.getTimeSignatureTrack();
    if (columnId == 1)
        g.drawText(String((*track)[rowNumber]->getId()), 2, 0, width - 4, height, Justification::centredLeft, true);
    else if (columnId == 3)
        g.drawText(String(project.getBarAndBeat((*track)[rowNumber]->getBeat()).first), 2, 0, width - 4, height, Justification::centredLeft, true);
    else if (columnId == 4)
        g.drawText(String(project.getBarAndBeat((*track)[rowNumber]->getBeat()).second, 2), 2, 0, width - 4, height, Justification::centredLeft, true);

    g.setColour(getLookAndFeel().findColour(ListBox::backgroundColourId));
    g.fillRect(width - 1, 0, 1, height);
}

Component* SimpleTimeSignatureTrackEditor::refreshComponentForCell(int rowNumber, int columnId, bool /*isRowSelected*/, Component* existingComponentToUpdate)
{
    if (columnId == 2 || columnId == 5 || columnId == 6)
    {
        auto* textLabel = static_cast<EditableTextCustomComponent*> (existingComponentToUpdate);
        if (textLabel == nullptr)
            textLabel = new EditableTextCustomComponent(*this);
        textLabel->setRowAndColumn(rowNumber, columnId);
        return textLabel;
    }

    jassert(existingComponentToUpdate == nullptr);
    return nullptr;
}

String SimpleTimeSignatureTrackEditor::getText(const int rowNumber, const int columnNumber) const
{
    auto* track = project.getTimeSignatureTrack();
    String text;
    switch (columnNumber)
    {
    case 2: text = String((*track)[rowNumber]->getBeat(), 2); break;
    case 5: text = String((*track)[rowNumber]->getNumerator()); break;
    case 6: text = String((*track)[rowNumber]->getDenominator()); break;
    default:
        break;
    }
    return text;
}

void SimpleTimeSignatureTrackEditor::setText(const int rowNumber, const int columnNumber, const String& newText)
{
    auto* track = project.getTimeSignatureTrack();
    auto* sig = (*track)[rowNumber];
    switch (columnNumber)
    {
    case 2: {
        if (track && sig->getBeat() != roundBeat(newText.getFloatValue()))
        {
            processor.getUndoManager().beginNewTransaction("'Change time sig. position'");
            track->change(*sig, (*sig).withBeat(newText.getFloatValue()), true);
        }
        break;
    }
    case 5: {
        if (track && sig->getNumerator() != newText.getIntValue())
        {
            processor.getUndoManager().beginNewTransaction("'Change time sig. numerator'");
            track->change(*sig, (*sig).withNumerator(newText.getIntValue()), true);
        }
        break;
    }
    case 6: {
        if (track && sig->getDenominator() != newText.getIntValue())
        {
            processor.getUndoManager().beginNewTransaction("'Change time sig. denominator'");
            track->change(*sig, (*sig).withDenominator(newText.getIntValue()), true);
        }
        break;
    }
    default:
        break;
    }
}

void SimpleTimeSignatureTrackEditor::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void SimpleTimeSignatureTrackEditor::resized()
{
    auto b = getLocalBounds();
    b.reduce(10, 10);
    auto half = b.proportionOfWidth(1.0 / 2);

    auto tmp = b.removeFromBottom(28);
    b.removeFromBottom(4);
    addBtn.setBounds(tmp.removeFromLeft(half).reduced(2));
    removeBtn.setBounds(tmp.removeFromLeft(half).reduced(2));

    table.setBounds(b);
}

//===------------------------------------------------------------------===//
// Project Listener
//===------------------------------------------------------------------===//
void SimpleTimeSignatureTrackEditor::onPostAddMidiEvent()
{
    table.updateContent();
    table.repaint();
}

void SimpleTimeSignatureTrackEditor::onPostChangeMidiEvent()
{
    table.updateContent();
    table.repaint();
}

void SimpleTimeSignatureTrackEditor::onPostRemoveMidiEvent(MidiTrack* const layer)
{
    table.updateContent();
    table.repaint();
}

void SimpleTimeSignatureTrackEditor::onReloadProjectContent(const Array<MidiTrack*>& tracks)
{
    table.updateContent();
    table.repaint();
}




SimpleTimeEditor::SimpleTimeEditor(MicroChromoAudioProcessorEditor& editor) : owner(editor)
{
    addAndMakeVisible(tempoEditor);
    addAndMakeVisible(timeSigEditor);
    setSize(305, 300);
}

//===------------------------------------------------------------------===//
// Components
//===------------------------------------------------------------------===//
void SimpleTimeEditor::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void SimpleTimeEditor::resized()
{
    auto b = getLocalBounds();
    tempoEditor.setBounds(b.removeFromTop(b.proportionOfHeight(0.5)));
    timeSigEditor.setBounds(b);
}
