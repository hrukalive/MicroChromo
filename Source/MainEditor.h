/*
  ==============================================================================

    MainEditor.h
    Created: 8 Apr 2020 9:50:26am
    Author:  bowen

  ==============================================================================
*/

#pragma once

#include "Common.h"
#include "ProjectListener.h"

class MidiTrack;
class PluginBundle;
class MicroChromoAudioProcessor;
class MicroChromoAudioProcessorEditor;

//==============================================================================
/*
*/
class MainEditor :
    public AudioProcessorEditor,
    public ChangeListener,
    public ProjectListener,
    public Timer,
    public DragAndDropContainer
{
public:
    MainEditor(MicroChromoAudioProcessor& p, MicroChromoAudioProcessorEditor& parent);
    ~MainEditor();

    //==============================================================================
    void paint(Graphics&) override;
    void resized() override;

    //==============================================================================
    void exportMidiDialog();
    void mouseDoubleClick(const MouseEvent& event) override;
    void mouseDrag(const MouseEvent&) override;

    //==============================================================================
    void changeListenerCallback(ChangeBroadcaster*) override;
    void timerCallback() override;
    void itemDroppedCallback(const Array<File>& files);

    //===------------------------------------------------------------------===//
    // Project Listener
    //===------------------------------------------------------------------===//
    void onAddMidiEvent(const MidiEvent& event) override {}
    void onPostAddMidiEvent() override {}
    void onChangeMidiEvent(const MidiEvent& oldEvent, const MidiEvent& newEvent) override {}
    void onPostChangeMidiEvent() override {}
    void onRemoveMidiEvent(const MidiEvent& event) override {}
    void onPostRemoveMidiEvent(MidiTrack* const layer) override {}

    void onAddTrack(MidiTrack* const track) override {}
    void onChangeTrackProperties(MidiTrack* const track) override {}
    void onRemoveTrack(MidiTrack* const track) override {}
    void onPostRemoveTrack() override {}

    void onAddPitchColorMapEntry(const PitchColorMapEntry& entry) override {}
    void onPostAddPitchColorMapEntry() override {}
    void onChangePitchColorMapEntry(const PitchColorMapEntry& oldEntry, const PitchColorMapEntry& newEntry) override {}
    void onPostChangePitchColorMapEntry() override {}
    void onRemovePitchColorMapEntry(const PitchColorMapEntry& entry) override {}
    void onPostRemovePitchColorMapEntry() override {}
    void onChangePitchColorMap(PitchColorMap* const colorMap) override {}

    void onChangeProjectBeatRange(float firstBeat, float lastBeat) override {}
    void onChangeViewBeatRange(float firstBeat, float lastBeat) override {}

    void onReloadProjectContent(const Array<MidiTrack*>& tracks) override;

    //==============================================================================

private:
    MicroChromoAudioProcessor& processor;
    MicroChromoAudioProcessorEditor& _parent;
    ApplicationProperties& appProperties;
    std::shared_ptr<PluginBundle> synthBundle, psBundle;

    friend class MicroChromoAudioProcessorEditor;

    class TextButtonDropTarget : public TextButton, public FileDragAndDropTarget
    {
    public:
        TextButtonDropTarget(String text, MainEditor& owner);
        ~TextButtonDropTarget() = default;

        bool isInterestedInFileDrag(const StringArray& files) override;
        void filesDropped(const StringArray& files, int x, int y) override;

    private:
        MainEditor& _owner;
    };

    std::unique_ptr<Button> synthButton, effectButton, dragButton, dropButton;
    std::unique_ptr<Label> synthLabel, effectLabel;
    std::unique_ptr<Label> numInstancesLabel, numParameterLabel, midiChannelLabel;
    std::unique_ptr<ComboBox> numInstancesBox, midiChannelComboBox;
    std::unique_ptr<TextEditor> numParameterSlot;
    std::unique_ptr<PopupMenu> floatMenu;

    String lastOpenedLocation = "", lastSavedLocation = "";

    std::atomic<bool> ignoreInitialChange1{ true }, ignoreInitialChange2{ true };
    bool canDrop = true;

    std::function<void(int, bool)> bundlePopupMenuSelected;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainEditor)
};