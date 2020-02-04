/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"


//==============================================================================
/**
*/
class MicroChromoAudioProcessorEditor  : 
	public AudioProcessorEditor, 
	public ChangeListener, 
	public Button::Listener
{
public:
    MicroChromoAudioProcessorEditor (MicroChromoAudioProcessor&, ApplicationProperties&, KnownPluginList&, AudioPluginFormatManager&);
    ~MicroChromoAudioProcessorEditor();

	//==============================================================================
	void changeListenerCallback(ChangeBroadcaster*) override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

	//==============================================================================
	void buttonClicked(Button* btn) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MicroChromoAudioProcessor& processor;
	ApplicationProperties& appProperties;
	AudioPluginFormatManager& formatManager;

	KnownPluginList& knownPluginList;
	KnownPluginList::SortMethod pluginSortMethod;
	Array<PluginDescription> pluginDescriptions;

	class PluginListWindow;
	std::unique_ptr<PluginListWindow> pluginListWindow;
	std::unique_ptr<Button> button1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MicroChromoAudioProcessorEditor)
};
