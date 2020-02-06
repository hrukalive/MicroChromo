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
	public ApplicationCommandTarget,
	public MenuBarModel,
	public Button::Listener
{
public:
	enum CommandIDs
	{
		openPluginScanner = 1,
		testCommand
	};

	//==============================================================================
    MicroChromoAudioProcessorEditor (MicroChromoAudioProcessor&, ApplicationProperties&, KnownPluginList&, AudioPluginFormatManager&);
    ~MicroChromoAudioProcessorEditor();

	//==============================================================================
	void changeListenerCallback(ChangeBroadcaster*) override;

	//==============================================================================
	StringArray getMenuBarNames() override;
	PopupMenu getMenuForIndex(int menuIndex, const String& /*menuName*/) override;
	void menuItemSelected(int /*menuItemID*/, int /*topLevelMenuIndex*/) override {};

	//==============================================================================
	ApplicationCommandTarget* getNextCommandTarget() override;
	void getAllCommands(Array<CommandID>& c) override;
	void getCommandInfo(CommandID commandID, ApplicationCommandInfo& result) override;
	bool perform(const InvocationInfo& info) override;

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
	ApplicationCommandManager commandManager;
	std::unique_ptr<MenuBarComponent> menuBar;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MicroChromoAudioProcessorEditor)
};
