/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "PluginWindow.h"

//==============================================================================
/**
*/
class MicroChromoAudioProcessorEditor  : 
	public AudioProcessorEditor,
	public ApplicationCommandTarget,
	public MenuBarModel,
	public ChangeListener,
	public Button::Listener
{
public:
	enum CommandIDs
	{
		openPluginScanner = 1,
		testCommand
	};

	//==============================================================================
    MicroChromoAudioProcessorEditor (MicroChromoAudioProcessor&);
    ~MicroChromoAudioProcessorEditor();

	//==============================================================================
	void changeListenerCallback(ChangeBroadcaster*) override;
	void mouseDown(const MouseEvent&) override;

	//==============================================================================
	StringArray getMenuBarNames() override;
	PopupMenu getMenuForIndex(int menuIndex, const String& /*menuName*/) override;
	void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

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
	void showPopupMenu(int type, Point<int> position, std::function<void(int)> callback);
	void graphUpdated();
	void showWindow(PluginWindow::Type type, AudioProcessorGraph::NodeID pluginID);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MicroChromoAudioProcessor& processor;
	ApplicationProperties& appProperties;
	AudioPluginFormatManager& formatManager;
	AudioProcessorGraph& mainProcessor;

	KnownPluginList& knownPluginList;
	KnownPluginList::SortMethod pluginSortMethod;

	class PluginListWindow;
	std::unique_ptr<PluginListWindow> pluginListWindow;
	std::vector<String> names;
	ApplicationCommandManager commandManager;
	std::unique_ptr<MenuBarComponent> menuBar;

	std::unique_ptr<Button> synthBtn, psBtn;
	std::unique_ptr<Label> synthLabel, psLabel;
	AudioProcessorGraph::NodeID synthId, psId;
	std::unique_ptr<PopupMenu> floatMenu;
	OwnedArray<PluginWindow> activePluginWindows;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MicroChromoAudioProcessorEditor)
};
