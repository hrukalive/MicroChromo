/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "Common.h"
#include "PluginProcessor.h"
#include "PluginInstance.h"
#include "PluginBundle.h"

//==============================================================================
/**
*/
class MicroChromoAudioProcessorEditor  : 
    public AudioProcessorEditor,
    public ApplicationCommandTarget,
    public MenuBarModel,
    public ChangeListener,
    public Button::Listener,
    public Timer,
    public DragAndDropContainer
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
    void mouseDrag(const MouseEvent&) override;

    //==============================================================================
    StringArray getMenuBarNames() override;
    PopupMenu getMenuForIndex(int menuIndex, const String& /*menuName*/) override;
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

    //==============================================================================
    ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands(Array<CommandID>& c) override;
    void getCommandInfo(CommandID commandID, ApplicationCommandInfo& result) override;
    bool perform(const InvocationInfo& info) override;
    MicroChromoAudioProcessor& getProcessor() { return processor; }

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    //==============================================================================
    void buttonClicked(Button* btn) override;
    //void showPopupMenu(std::function<void(int, bool)> callback, bool isSynth);

    void timerCallback() override;

private:
    MicroChromoAudioProcessor& processor;
    ApplicationProperties& appProperties;
    AudioPluginFormatManager& formatManager;
    std::shared_ptr<PluginBundle> synthBundle, psBundle;
    std::atomic<bool> ignoreInitialChange{ true };

    bool test = true;
    int lastNote = 60;
    int lastNote2 = 60;

    KnownPluginList& knownPluginList;
    KnownPluginList::SortMethod pluginSortMethod;

    class PluginListWindow;
    std::unique_ptr<PluginListWindow> pluginListWindow;
    std::vector<String> names;
    ApplicationCommandManager commandManager;
    std::unique_ptr<MenuBarComponent> menuBar;

    std::unique_ptr<Button> synthBtn, psBtn, noteButton, ccLearnBtn, dragBtn;
    std::unique_ptr<Label> synthLabel, psLabel;
    std::unique_ptr<PopupMenu> floatMenu;

    Label transportLabel;

    Label numInstancesLabel;
    ComboBox numInstancesBox;
    TextEditor numParameterSlot;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MicroChromoAudioProcessorEditor)
};
