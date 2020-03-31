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
    void focusGained(FocusChangeType cause) override;

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
    KnownPluginList::SortMethod getPluginSortMethod() { return pluginSortMethod; }

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    void currentTabChanged(int newCurrentTabIndex);

private:
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
        void changeListenerCallback(ChangeBroadcaster*) override;
        void mouseDown(const MouseEvent&) override;
        void mouseDrag(const MouseEvent&) override;

        //==============================================================================
        void paint(Graphics&) override;
        void resized() override;

        //==============================================================================
        void timerCallback() override;

    private:
        MicroChromoAudioProcessor& processor;
        MicroChromoAudioProcessorEditor& _parent;
        ApplicationProperties& appProperties;
        std::shared_ptr<PluginBundle> synthBundle, psBundle;

        std::unique_ptr<TextButton> synthButton, effectButton, dragButton;
        std::unique_ptr<TextButton> noteBtn;
        std::unique_ptr<Label> synthLabel, effectLabel;
        std::unique_ptr<Label> numInstancesLabel, numParameterLabel;
        std::unique_ptr<ComboBox> numInstancesBox;
        std::unique_ptr<TextEditor> numParameterSlot;
        std::unique_ptr<PopupMenu> floatMenu;

        std::atomic<bool> ignoreInitialChange{ true };
        bool test = true;
        int lastNote = 60;
        int lastNote2 = 60;

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

    MicroChromoAudioProcessor& processor;
    ApplicationProperties& appProperties;
    AudioPluginFormatManager& formatManager;
    ApplicationCommandManager commandManager;
    std::shared_ptr<PluginBundle> synthBundle, psBundle;

    KnownPluginList& knownPluginList;
    KnownPluginList::SortMethod pluginSortMethod;

    std::unique_ptr<Label> centerMessageLabel;

    class CustomTabbedButtonComponent : public TabbedButtonBar
    {
    public:
        CustomTabbedButtonComponent(MicroChromoAudioProcessorEditor& parent, TabbedButtonBar::Orientation orientation);
        ~CustomTabbedButtonComponent() = default;

        void currentTabChanged(int newCurrentTabIndex, const String&) override;
    private:
        MicroChromoAudioProcessorEditor& _parent;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomTabbedButtonComponent)
    };
    std::unique_ptr<CustomTabbedButtonComponent> tabbedButtons;
    std::unique_ptr<MainEditor> mainEditor;
    std::unique_ptr<MenuBarComponent> menuBar;

    std::unique_ptr<AudioProcessorEditor> synthUi{ nullptr }, effectUi{ nullptr };
    int synthWidth, synthHeight, effectWidth, effectHeight;
    bool synthBeforeEffect = false, effectBeforeSynth = false;

    class MainViewport : public Component
    {
    public:
        MainViewport(int size)
        {
            comps.resize(size);
            comps.fill(nullptr);
        }
        ~MainViewport() = default;

        void setUI(int i, Component* ui)
        {
            if (comps[i])
                removeChildComponent(comps[i]);
            if (ui == nullptr)
            {
                comps.set(i, nullptr);
                return;
            }
            comps.set(i, ui);
            addChildComponent(ui);
            ui->setVisible(false);
        }

        void showUI(int i)
        {
            for (auto* w : comps)
                if (w)
                    w->setVisible(false);
            comps[i]->setVisible(true);
            comps[i]->toFront(true);
        }

        void paint(Graphics& g) override
        {
            g.fillAll(Colour::fromRGBA(220, 50, 200, 100));
        }
        void resized() override
        {
            auto b = getLocalBounds();
            for (auto* w : comps)
                if (w)
                    w->setBounds(b);
        }

    private:
        Array<Component*> comps;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainViewport)
    };
    std::unique_ptr<MainViewport> mainComp;

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MicroChromoAudioProcessorEditor)
};
