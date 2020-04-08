/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "Common.h"
#include "Note.h"
#include "PluginInstance.h"
#include "PluginBundle.h"

using GUICallback = std::function<void()>;

//==============================================================================
/**
*/
class MicroChromoAudioProcessor  : public AudioProcessor, ChangeListener
{
public:
    //==============================================================================
    MicroChromoAudioProcessor();
    ~MicroChromoAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void addPlugin(const PluginDescription& desc, bool isSynth, std::function<void(PluginBundle&)> callback = nullptr);
    void startLoadingPlugin();
    void finishLoadingPlugin();

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void changeListenerCallback(ChangeBroadcaster* changed) override;
    void adjustInstanceNumber(int newNumInstances);

    void updateMidiSequence();
    void updateMidiSequenceGeneral();
    void updateMidiSequenceKontakt();
    void updateCcMidiSequenceWithNewBase(int newBase);
    void sendAllNotesOff();

    //==============================================================================
    void addNote(const Note& note);
    void clearNotes();
    void insertNote(int index, const Note& note);

    //==============================================================================
    ApplicationProperties& getApplicationProperties() { return appProperties; }
    AudioPluginFormatManager& getAudioPluginFormatManager() { return formatManager; }
    KnownPluginList& getKnownPluginList() { return knownPluginList; }
    KnownPluginList& getSynthKnownPluginList() { return synthKnownPluginList; }
    KnownPluginList& getPsKnownPluginList() { return psKnownPluginList; }
    std::shared_ptr<PluginBundle> getSynthBundlePtr() { return synthBundle; }
    std::shared_ptr<PluginBundle> getPSBundlePtr() { return psBundle; }
    int getNumInstances() { return numInstancesParameter; }
    AudioProcessorValueTreeState& getValueTreeState() { return parameters; }
    UndoManager* getUndoManager() noexcept { return &undoManager; }
    int getParameterSlotNumber() { return parameterSlotNumber; }
    int getMidiChannel() { return midiChannel; }
    int getCcBase() { return ccBase; }

    OwnedArray<MidiMessageSequence>& getNoteMidiSequence() { return notesMidiSeq; }
    OwnedArray<MidiMessageSequence>& getCcMidiSequence() { return ccMidiSeq; }

    void updatePitchShiftModulationSource();
    int getPitchShiftModulationSource();
    bool canLearnCc(const PluginBundle* bundle);
    bool canChooseKontakt();
    void useKontakt();

    void updateMidiChannel(int newMidiChannel);
    void updateKontaktCcBase(int newCcBase);

    static const int MAX_INSTANCES = 8;
private:
    //==============================================================================
    ApplicationProperties appProperties;
    KnownPluginList knownPluginList, synthKnownPluginList, psKnownPluginList;
    AudioPluginFormatManager formatManager;
    Array<PluginDescription> internalTypes;
    AudioProcessorValueTreeState parameters;
    UndoManager undoManager;

    //==============================================================================
    std::atomic<int> numInstancesParameter{ 1 }, parameterSlotNumber{ 16 };

    OwnedArray<AudioBuffer<float>> bufferArrayA, bufferArrayB;
    std::shared_ptr<PluginBundle> synthBundle, psBundle;
    std::shared_ptr<PluginBundle> ccBundle{ nullptr };

    OwnedArray<ParameterLinker> synthParamPtr, psParamPtr;

    Array<Note> notes;
    OwnedArray<MidiMessageSequence> notesMidiSeq, ccMidiSeq;

    std::atomic<int> ccBase{ 102 }, psModSource{ -1 }, midiChannel{ 1 };

    //==============================================================================
    AudioPlayHead::CurrentPositionInfo posInfo;
    Array<MidiMessage> controllerStateMessage;

    std::atomic<bool> properlyPrepared{ false };

    std::atomic<bool> isPlayingNote{ false };
    std::atomic<int> isWithIn{ -1 };

    float nextStartTime = -1.0, rangeStartTime = FP_INFINITE, rangeEndTime = -FP_INFINITE;

    int synthBundleTotalNumInputChannels, synthBundleMainBusNumInputChannels, synthBundleMainBusNumOutputChannels,
        psBundleTotalNumInputChannels, psBundleTotalNumOutputChannels, psBundleMainBusNumInputChannels, psBundleMainBusNumOutputChannels;
    float sampleLength{ -1 }, bufferLength{ -1 };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MicroChromoAudioProcessor)
};
