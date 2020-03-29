#pragma once

#include <JuceHeader.h>

class PluginBundle;

class ParameterLinkEditor  : public Component, public ListBoxModel
{
public:
    //==============================================================================
    ParameterLinkEditor (PluginBundle& bundle);
    ~ParameterLinkEditor() override;

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;

    //==============================================================================
    int getNumRows() override { return parameters.size(); }
    void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const MouseEvent& e) override;

private:

    //==============================================================================
    ListBox parameterList;
    Label statusLabel;
    TextButton okBtn{ "OK" };
    PluginBundle& _bundle;

    const int numParameterSlot;
    Array<std::pair<AudioProcessorParameter*, int>> parameters;
    std::unordered_set<int> selectedSet;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterLinkEditor)
};
