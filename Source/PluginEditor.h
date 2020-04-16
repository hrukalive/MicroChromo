/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "Common.h"

class MicroChromoAudioProcessor;
class MainEditor;
class SimpleMidiEditor;
class ColorEditor;

//==============================================================================
class MicroChromoAudioProcessorEditor  : 
    public AudioProcessorEditor,
    public ApplicationCommandTarget,
    public MenuBarModel,
    public ChangeListener,
    public Timer
{
public:

    //==============================================================================
    MicroChromoAudioProcessorEditor (MicroChromoAudioProcessor&);
    ~MicroChromoAudioProcessorEditor();

    //==============================================================================
    void paint(Graphics&) override;
    void resized() override;

    //==============================================================================
    ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands(Array<CommandID>& c) override;
    void getCommandInfo(CommandID commandID, ApplicationCommandInfo& result) override;
    bool perform(const InvocationInfo& info) override;

    MicroChromoAudioProcessor& getProcessor() { return processor; }
    KnownPluginList::SortMethod getPluginSortMethod() { return pluginSortMethod; }

    //==============================================================================
    StringArray getMenuBarNames() override;
    PopupMenu getMenuForIndex(int menuIndex, const String& /*menuName*/) override;
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

    //==============================================================================
    bool keyPressed(const KeyPress& key) override;
    void changeListenerCallback(ChangeBroadcaster*) override;
    void timerCallback() override;

    void updateMidiEditor();

private:

    MicroChromoAudioProcessor& processor;
    ApplicationProperties& appProperties;
    AudioPluginFormatManager& formatManager;
    ApplicationCommandManager commandManager;
    UndoManager& undoManager;
    std::shared_ptr<PluginBundle> synthBundle, psBundle;

    KnownPluginList& knownPluginList;
    KnownPluginList::SortMethod pluginSortMethod;

    std::unique_ptr<MainEditor> mainEditor;
    std::unique_ptr<SimpleMidiEditor> midiEditor;
    std::unique_ptr<ColorEditor> colorEditor;

    std::unique_ptr<MenuBarComponent> menuBar;
    TextButton playTransportBtn{ "Play" }, stopTransportBtn{ "Stop" }, panicBtn{ "!" };
    Slider transportSlider;
    Label transportLabel;
    std::atomic<bool> transportSliderDragging{ false };

    class CustomTabbedComponent : public TabbedComponent
    {
    public:
        CustomTabbedComponent(TabbedButtonBar::Orientation orientation, MicroChromoAudioProcessorEditor& owner);
        ~CustomTabbedComponent() = default;

        virtual void currentTabChanged(int newCurrentTabIndex, const String& /*newCurrentTabName*/) override;

    private:
        MicroChromoAudioProcessorEditor& _owner;
    };
    std::unique_ptr<CustomTabbedComponent> tabComp;

    class PluginListWindow : public DocumentWindow
    {
    public:
        PluginListWindow(MicroChromoAudioProcessorEditor& mw, AudioPluginFormatManager& pluginFormatManager);
        ~PluginListWindow();
        void closeButtonPressed();
    private:
        MicroChromoAudioProcessorEditor& owner;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginListWindow)
    };
    std::unique_ptr<PluginListWindow> pluginListWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MicroChromoAudioProcessorEditor)
};
