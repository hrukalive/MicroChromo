/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginWindow.h"
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

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    //==============================================================================
    void buttonClicked(Button* btn) override;
    void showPopupMenu(int type, Point<int> position, std::function<void(int)> callback);
    void showWindow(PluginWindow::Type type, bool isSynth);

    void timerCallback() override;

private:
    MicroChromoAudioProcessor& processor;
    ApplicationProperties& appProperties;
    AudioPluginFormatManager& formatManager;
    std::shared_ptr<PluginBundle> synthBundle, psBundle;

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
    OwnedArray<PluginWindow> activePluginWindows;

    Label numInstancesLabel;
    ComboBox numInstancesBox;

    foleys::LevelMeterLookAndFeel lnf;
    foleys::LevelMeter meterInput{ foleys::LevelMeter::MeterFlags::Default };
    foleys::LevelMeter meterOutput{ foleys::LevelMeter::MeterFlags::Default };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MicroChromoAudioProcessorEditor)
};
