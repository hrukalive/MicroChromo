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
class MicroChromoAudioProcessorEditor  : 
    public AudioProcessorEditor,
    public ApplicationCommandTarget,
    public MenuBarModel,
    public ChangeListener
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
    void changeListenerCallback(ChangeBroadcaster*) override;

private:

    MicroChromoAudioProcessor& processor;
    ApplicationProperties& appProperties;
    AudioPluginFormatManager& formatManager;
    ApplicationCommandManager commandManager;
    std::shared_ptr<PluginBundle> synthBundle, psBundle;

    KnownPluginList& knownPluginList;
    KnownPluginList::SortMethod pluginSortMethod;

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

        std::function<void(int, bool)> bundlePopupMenuSelected = [this](int r, bool isSynth) {
            auto bundle = isSynth ? this->synthBundle : this->psBundle;
            switch (r)
            {
            case SLOT_MENU_LOAD_EMPTY_PLUGIN:
            {
                bundle->closeAllWindows();
                processor.addPlugin(bundle->getEmptyPluginDescription(), isSynth);
                break;
            }
            case SLOT_MENU_LOAD_DEFAULT_PLUGIN:
            {
                bundle->closeAllWindows();
                processor.addPlugin(bundle->getDefaultPluginDescription(), isSynth);
                break;
            }
            default:
            {
                if (r >= pluginMenuIdBase)
                {
                    bundle->closeAllWindows();
                    auto types = (isSynth ? processor.getSynthKnownPluginList() : processor.getPsKnownPluginList()).getTypes();
                    int result = KnownPluginList::getIndexChosenByMenu(types, r);
                    auto& desc = types.getReference(result);
                    processor.addPlugin(desc, isSynth);
                }
            }
            }
        };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainEditor)
    };
    std::unique_ptr<MainEditor> mainEditor;
    std::unique_ptr<MenuBarComponent> menuBar;

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

    class MidiEditorWindow : public DocumentWindow
    {
    public:
        MidiEditorWindow(MicroChromoAudioProcessorEditor& mw);
        ~MidiEditorWindow();
        void closeButtonPressed();
    private:
        MicroChromoAudioProcessorEditor& owner;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiEditorWindow)
    };
    std::unique_ptr<MidiEditorWindow> midiEditorWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MicroChromoAudioProcessorEditor)
};
