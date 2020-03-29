#pragma once
#include <JuceHeader.h>
#include "PluginInstance.h"

/**
    A window that shows a log of parameter change messagse sent by the plugin.
*/
class PluginDebugWindow : public AudioProcessorEditor,
                          public AudioProcessorParameter::Listener,
                          public ListBoxModel,
                          public AsyncUpdater
{
public:
    PluginDebugWindow (AudioProcessor& proc)
        : AudioProcessorEditor (proc), audioProc (proc)
    {
        setSize (500, 200);
        setAlwaysOnTop(true);

        addAndMakeVisible (list);

        for (auto* p : audioProc.getParameters())
            p->addListener (this);

        log.add ("Parameter debug log started");
    }

    ~PluginDebugWindow()
    {
        for (auto* p : audioProc.getParameters())
            p->removeListener(this);
    }

    void parameterValueChanged (int parameterIndex, float newValue) override
    {
        auto* param = audioProc.getParameters()[parameterIndex];
        auto value = param->getCurrentValueAsText().quoted() + " (" + String (newValue, 4) + ")";

        appendToLog ("parameter change", *param, value);
    }

    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override
    {
        auto* param = audioProc.getParameters()[parameterIndex];
        appendToLog ("gesture", *param, gestureIsStarting ? "start" : "end");
    }

private:
    void appendToLog (StringRef action, AudioProcessorParameter& param, StringRef value)
    {
        String entry (action + " " + param.getName (30).quoted() + " [" + String (param.getParameterIndex()) + "]: " + value);

        {
            ScopedLock lock (pendingLogLock);
            pendingLogEntries.add (entry);
        }

        triggerAsyncUpdate();
    }

    void resized() override
    {
        list.setBounds(getLocalBounds());
    }

    int getNumRows() override
    {
        return log.size();
    }

    void paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool) override
    {
        g.setColour (getLookAndFeel().findColour (TextEditor::textColourId));

        if (isPositiveAndBelow (rowNumber, log.size()))
            g.drawText (log[rowNumber], Rectangle<int> { 0, 0, width, height }, Justification::left, true);
    }

    void handleAsyncUpdate() override
    {
        if (log.size() > logSizeTrimThreshold)
            log.removeRange (0, log.size() - maxLogSize);

        {
            ScopedLock lock (pendingLogLock);
            log.addArray (pendingLogEntries);
            pendingLogEntries.clear();
        }

        list.updateContent();
        list.scrollToEnsureRowIsOnscreen (log.size() - 1);
    }

    JUCE_CONSTEXPR static const int maxLogSize = 300;
    JUCE_CONSTEXPR static const int logSizeTrimThreshold = 400;

    ListBox list { "Log", this };

    StringArray log;
    StringArray pendingLogEntries;
    CriticalSection pendingLogLock;

    AudioProcessor& audioProc;
};

//==============================================================================
/**
    A desktop window containing a plugin's GUI.
*/
class PluginWindow  : public DocumentWindow
{
public:
    enum class Type
    {
        normal = 0,
        generic,
        programs,
        debug,
        numTypes
    };

    PluginWindow (PluginInstance* n, Type t, OwnedArray<PluginWindow>& windowList)
       : DocumentWindow (n->processor->getName(),
                         LookAndFeel::getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId),
                         DocumentWindow::minimiseButton | DocumentWindow::closeButton),
         activeWindowList (windowList),
         node(n), type (t)
    {
        setSize (400, 300);
        setAlwaysOnTop(true);

        if (auto* ui = createProcessorEditor (*node->processor, type))
            setContentOwned (ui, true);
        setTopLeftPosition (node->properties.getWithDefault (getLastXProp (type), Random::getSystemRandom().nextInt (500)),
                            node->properties.getWithDefault (getLastYProp (type), Random::getSystemRandom().nextInt (500)));

        node->properties.set (getOpenProp (type), true);

        setVisible (true);
    }

    ~PluginWindow() override
    {
        clearContentComponent();
    }

    void moved() override
    {
        node->properties.set (getLastXProp (type), getX());
        node->properties.set (getLastYProp (type), getY());
    }

    void closeButtonPressed() override
    {
        node->properties.set (getOpenProp (type), false);
        activeWindowList.removeObject (this);
    }

    static String getLastXProp (Type type)    { return "uiLastX_" + getTypeName (type); }
    static String getLastYProp (Type type)    { return "uiLastY_" + getTypeName (type); }
    static String getOpenProp  (Type type)    { return "uiopen_"  + getTypeName (type); }

    OwnedArray<PluginWindow>& activeWindowList;
    PluginInstance* node;
    const Type type;

private:
    float getDesktopScaleFactor() const override     { return 1.0f; }

    static AudioProcessorEditor* createProcessorEditor (AudioProcessor& processor,
                                                        PluginWindow::Type type)
    {
        if (type == PluginWindow::Type::normal)
        {
            if (auto* ui = processor.createEditorIfNeeded())
                return ui;

            type = PluginWindow::Type::generic;
        }

        if (type == PluginWindow::Type::generic)  return new GenericAudioProcessorEditor (processor);
        if (type == PluginWindow::Type::programs) return new ProgramAudioProcessorEditor (processor);
        if (type == PluginWindow::Type::debug)    return new PluginDebugWindow (processor);

        jassertfalse;
        return {};
    }

    static String getTypeName (Type type)
    {
        switch (type)
        {
            case Type::normal:     return "Normal";
            case Type::generic:    return "Generic";
            case Type::programs:   return "Programs";
            case Type::debug:      return "Debug";
            default:               return {};
        }
    }

    //==============================================================================
    struct ProgramAudioProcessorEditor  : public AudioProcessorEditor
    {
        ProgramAudioProcessorEditor (AudioProcessor& p)  : AudioProcessorEditor (p)
        {
            setOpaque (true);
            setAlwaysOnTop(true);

            addAndMakeVisible (panel);

            Array<PropertyComponent*> programs;

            auto numPrograms = p.getNumPrograms();
            int totalHeight = 0;

            for (int i = 0; i < numPrograms; ++i)
            {
                auto name = p.getProgramName (i).trim();

                if (name.isEmpty())
                    name = "Unnamed";

                auto pc = new PropertyComp (name, p);
                programs.add (pc);
                totalHeight += pc->getPreferredHeight();
            }

            panel.addProperties (programs);

            setSize (400, jlimit (25, 400, totalHeight));
        }

        void paint (Graphics& g) override
        {
            g.fillAll (Colours::grey);
        }

        void resized() override
        {
            panel.setBounds (getLocalBounds());
        }

    private:
        struct PropertyComp  : public PropertyComponent,
                               private AudioProcessorListener
        {
            PropertyComp (const String& name, AudioProcessor& p)  : PropertyComponent (name), owner (p)
            {
                owner.addListener (this);
            }

            ~PropertyComp() override
            {
                owner.removeListener (this);
            }

            void refresh() override {}
            void audioProcessorChanged (AudioProcessor*) override {}
            void audioProcessorParameterChanged (AudioProcessor*, int, float) override {}

            AudioProcessor& owner;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyComp)
        };

        PropertyPanel panel;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProgramAudioProcessorEditor)
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginWindow)
};
