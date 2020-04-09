/*
  ==============================================================================

    MainEditor.h
    Created: 8 Apr 2020 9:50:26am
    Author:  bowen

  ==============================================================================
*/

#pragma once

#include "Common.h"

class MicroChromoAudioProcessor;
class MicroChromoAudioProcessorEditor;

//==============================================================================
/*
*/
class MainEditor :
    public AudioProcessorEditor,
    public ChangeListener,
    public Timer,
    public DragAndDropContainer
{
public:
    MainEditor(MicroChromoAudioProcessor& p, MicroChromoAudioProcessorEditor& parent);
    ~MainEditor();

    //==============================================================================
    void paint(Graphics&) override;
    void resized() override;

    void mouseDown(const MouseEvent&) override;
    void mouseDrag(const MouseEvent&) override;

    //==============================================================================
    void changeListenerCallback(ChangeBroadcaster*) override;

    //==============================================================================
    void timerCallback() override;

private:
    MicroChromoAudioProcessor& processor;
    MicroChromoAudioProcessorEditor& _parent;
    ApplicationProperties& appProperties;
    std::shared_ptr<PluginBundle> synthBundle, psBundle;

    std::unique_ptr<TextButton> synthButton, effectButton, dragButton;
    std::unique_ptr<Label> synthLabel, effectLabel;
    std::unique_ptr<Label> numInstancesLabel, numParameterLabel, midiChannelLabel;
    std::unique_ptr<ComboBox> numInstancesBox, midiChannelComboBox;
    std::unique_ptr<TextEditor> numParameterSlot;
    std::unique_ptr<PopupMenu> floatMenu;

    std::atomic<bool> ignoreInitialChange1{ true }, ignoreInitialChange2{ true };
    bool canDrop = true;

    std::function<void(int, bool)> bundlePopupMenuSelected;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainEditor)
};