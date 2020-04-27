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

#include "ParameterLinkEditor.h"

#include "PluginProcessor.h"
#include "PluginBundle.h"
#include "ParameterCcLearn.h"

//==============================================================================
ParameterLinkEditor::ParameterLinkEditor (PluginBundle& bundle) : 
    _bundle(bundle), numParameterSlot(bundle.getProcessor().getParameterSlotNumber())
{
    int j = 0;
    auto learnedCc = bundle.getCcLearnModule().getCcLearnedParameterIndex();
    for (auto* p : bundle.getParameters())
    {
        if (!learnedCc.contains(j))
            parameters.add(std::make_pair(p, j));
        j++;
    }

    parameterList.setModel(this);
    addAndMakeVisible (parameterList);
    parameterList.setColour(ListBox::backgroundColourId, Colours::black.withAlpha(0.02f));
    parameterList.setColour(ListBox::outlineColourId, Colours::black.withAlpha(0.1f));
    parameterList.setOutlineThickness(1);

    for (int i : bundle.getLinkedArray())
        selectedSet.insert(i);

    addAndMakeVisible (statusLabel);
    statusLabel.setText(String(selectedSet.size()) + " of " + String(parameters.size()) + " selected", dontSendNotification);

    addAndMakeVisible(okBtn);
    okBtn.onClick = [this, &bundle]()
    {
        Array<int> indices;
        for (int i : selectedSet)
            indices.add(i);
        bundle.linkEditorExit(indices);
        this->getTopLevelComponent()->exitModalState(0);
    };

    setSize (300, 400);
}

//===------------------------------------------------------------------===//
// ListBoxModel
//===------------------------------------------------------------------===//
void ParameterLinkEditor::paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool /*rowIsSelected*/)
{
    if (selectedSet.contains(parameters[rowNumber].second))
        g.fillAll(findColour(TextEditor::highlightColourId));

    g.setColour(findColour(ListBox::textColourId));
    Font f(height * 0.7f);
    f.setHorizontalScale(0.9f);
    g.setFont(f);

    g.drawText(parameters[rowNumber].first->getName(128),
        4, 0, width - 6, height,
        Justification::centredLeft, true);
}

void ParameterLinkEditor::listBoxItemClicked(int row, const MouseEvent&)
{
    if (selectedSet.contains(parameters[row].second))
        selectedSet.erase(selectedSet.find(parameters[row].second));
    else
        selectedSet.insert(parameters[row].second);
    parameterList.repaintRow(row);
    statusLabel.setText(String(selectedSet.size()) + " of " + String(parameters.size()) + " selected", dontSendNotification);
}

//===------------------------------------------------------------------===//
// Components
//===------------------------------------------------------------------===//
void ParameterLinkEditor::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void ParameterLinkEditor::resized()
{
    auto b = getLocalBounds();
    b.reduce(10, 10);
    okBtn.setBounds(b.removeFromBottom(40));
    b.removeFromBottom(10);
    statusLabel.setBounds(b.removeFromBottom(24));
    b.removeFromBottom(10);
    parameterList.setBounds(b);
}
