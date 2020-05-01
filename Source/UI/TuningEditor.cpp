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

#include "TuningEditor.h"

#include "PluginProcessor.h"
#include "PluginEditor.h"

TuningEditor::TuningEditor(MicroChromoAudioProcessorEditor& editor) :
    owner(editor), processor(editor.getProcessor()),
    project(editor.getProcessor().getProject())
{
    for (int i = 0; i < 12; i++)
    {
        addAndMakeVisible(noteLabels[i]);
        noteLabels[i].setText(MidiMessage::getMidiNoteName(i, true, false, 4), dontSendNotification);

        addAndMakeVisible(noteSliders[i]);
        noteSliders[i].setRange(-100, 100, 1);
        noteSliders[i].setValue(project.getTuning(i), dontSendNotification);
        noteSliders[i].setSliderStyle(Slider::SliderStyle::LinearHorizontal);
        noteSliders[i].onDragEnd = [i, this]() {
            this->project.getUndoManager().beginNewTransaction("'Change tuning of note " + MidiMessage::getMidiNoteName(i, true, false, 4) + "'");
            this->project.changeTuning(i, noteSliders[i].getValue(), true);
        };
    }

    addAndMakeVisible(freqALabel);
    freqALabel.setText("A4 Freq", dontSendNotification);

    addAndMakeVisible(freqASlider);
    freqASlider.setRange(400, 480, 1);
    freqASlider.setValue(project.getFreqOfA(), dontSendNotification);
    freqASlider.setSliderStyle(Slider::SliderStyle::LinearHorizontal);
    freqASlider.onDragEnd = [&]() {
        project.getUndoManager().beginNewTransaction("'Change pitch standard'");
        project.changeFreqOfA(freqASlider.getValue(), true);
    };

    addAndMakeVisible(resetBtn);
    resetBtn.onClick = [&]() {
        project.getUndoManager().beginNewTransaction("'Reset tuning'");
        for (int i = 0; i < 12; i++)
            project.changeTuning(i, 0, true);
        project.changeFreqOfA(440, true);
    };

    project.addListener(this);
    setSize(350, 580);
}

TuningEditor::~TuningEditor()
{
    project.removeListener(this);
}

//===------------------------------------------------------------------===//
// Project Listener
//===------------------------------------------------------------------===//
void TuningEditor::onPostTuningChange()
{
    freqASlider.setValue(project.getFreqOfA(), dontSendNotification);
    for (int i = 0; i < 12; i++)
        noteSliders[i].setValue(project.getTuning(i), dontSendNotification);
}

void TuningEditor::onReloadProjectContent(const Array<MidiTrack*>& tracks)
{
    freqASlider.setValue(project.getFreqOfA(), dontSendNotification);
    for (int i = 0; i < 12; i++)
        noteSliders[i].setValue(project.getTuning(i), dontSendNotification);
}

//===------------------------------------------------------------------===//
// Components
//===------------------------------------------------------------------===//
void TuningEditor::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void TuningEditor::resized()
{
    auto b = getLocalBounds();
    b.reduce(10, 10);
    auto tmpHeight = b.proportionOfHeight(1 / 14.0f);

    resetBtn.setBounds(b.removeFromBottom(24));
    b.removeFromBottom(6);

    auto tmp = b.removeFromTop(tmpHeight);
    freqASlider.setBounds(tmp.removeFromRight(b.proportionOfWidth(0.8)));
    freqALabel.setBounds(tmp);
    auto tmpWidth = b.proportionOfWidth(0.9);
    b.removeFromTop(6);

    for (int i = 0; i < 12; i++)
    {
        tmp = b.removeFromTop(tmpHeight);
        noteSliders[i].setBounds(tmp.removeFromRight(tmpWidth));
        noteLabels[i].setBounds(tmp);
    }
}
