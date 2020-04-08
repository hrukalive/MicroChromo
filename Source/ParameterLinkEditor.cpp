/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 5.4.7

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library.
  Copyright (c) 2017 - ROLI Ltd.

  ==============================================================================
*/

#include "ParameterLinkEditor.h"
#include "PluginBundle.h"
#include "PluginProcessor.h"

//==============================================================================
ParameterLinkEditor::ParameterLinkEditor (PluginBundle& bundle) : 
    _bundle(bundle), numParameterSlot(bundle.getProcessor().getParameterSlotNumber())
{
    int j = 0;
    auto learnedCc = bundle.getLearnedCcParameterIndex();
    for (auto* p : bundle.getParameters())
    {
        if (j != learnedCc)
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

//==============================================================================
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

//==============================================================================
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
