#pragma once

#include <JuceHeader.h>

class PluginBundle;

class ParameterLinkEditor  : public Component, public ListBoxModel
{
public:
    //==============================================================================
    ParameterLinkEditor (PluginBundle& bundle);
    ~ParameterLinkEditor() = default;

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;

    //==============================================================================
    int getNumRows() override { return parameters.size(); }
    void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const MouseEvent& e) override;

private:
    //==============================================================================
    PluginBundle& _bundle;
    Array<std::pair<AudioProcessorParameter*, int>> parameters;

    const int numParameterSlot;
    std::unordered_set<int> selectedSet;

    ListBox parameterList;
    Label statusLabel;
    TextButton okBtn{ "OK" };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterLinkEditor)
};
