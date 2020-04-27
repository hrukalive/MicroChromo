/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "Common.h"
#include "PluginInstance.h"
#include "PluginBundle.h"
#include "Project.h"
#include "ParameterLinker.h"
#include "Note.h"

//==============================================================================
/**
*/
class MicroChromoAudioProcessor : public AudioProcessor, public ChangeListener, public FileBasedDocument, public HighResolutionTimer
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
    void hiResTimerCallback() override;
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
    void reset();
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void changeListenerCallback(ChangeBroadcaster* changed) override;
    void adjustInstanceNumber(int newNumInstances);

    void updateMidiSequence(int newBase = -1);
    void updateCcMidiSequenceWithNewBase(int newBase);

    //==============================================================================
    void togglePlayback();
    void stopPlayback();
    void setTimeForPlayback(double time);
    int getTransportState();
    double getTimeElapsed();

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
    UndoManager& getUndoManager() noexcept { return undoManager; }
    int getParameterSlotNumber() { return parameterSlotNumber; }
    int getMidiChannel() { return midiChannel; }
    int getCcBase() { return ccBase; }
    String getSelectedColorPresetName() { return selectedPreset; }
    void setSelectedColorPresetName(String name) { selectedPreset = name; }
    double getMidiSequenceEndTime() { return rangeEndTime; }

    Project& getProject() { return project; }

    OwnedArray<MidiMessageSequence>& getNoteMidiSequence() { return notesMidiSeq; }
    OwnedArray<MidiMessageSequence>& getCcMidiSequence() { return ccMidiSeq; }

    void updatePitchShiftModulationSource(int useKontakt = 0);
    int getPitchShiftModulationSource();
    bool canLearnCc(const PluginBundle* bundle);
    bool canChooseKontakt();
    void toggleUseKontakt(bool isOn);

    void updateMidiChannel(int newMidiChannel);
    void updateKontaktCcBase(int newCcBase);

    void triggerPanic() { panicNoteOff = true; }

    static const int MAX_INSTANCES = 8;

private:
    void addMessageToAllBuffer(OwnedArray<MidiBuffer>& midiBuffers, MidiMessage& msg, int sampleOffset);
    void sendAllNotesOff();
    void sendAllNotesOffPanic();

    //===------------------------------------------------------------------===//
    // FileBasedDocument
    //===------------------------------------------------------------------===//
    String getDocumentTitle() override;
    Result loadDocument(const File& file) override;
    Result saveDocument(const File& file) override;
    File getLastDocumentOpened() override;
    void setLastDocumentOpened(const File& file) override;

    //==============================================================================
    ApplicationProperties appProperties;
    KnownPluginList knownPluginList, synthKnownPluginList, psKnownPluginList;
    AudioPluginFormatManager formatManager;
    Array<PluginDescription> internalTypes;
    AudioProcessorValueTreeState parameters;
    UndoManager undoManager;

    //==============================================================================
    std::atomic<int> numInstancesParameter{ 1 }, parameterSlotNumber{ 16 };
    String selectedPreset = "---INIT---";

    //==============================================================================
    OwnedArray<AudioBuffer<float>> audioBufferArrayA, audioBufferArrayB;
    OwnedArray<MidiBuffer> midiBufferArrayA, midiBufferArrayB;
    std::shared_ptr<PluginBundle> synthBundle, psBundle;

    OwnedArray<ParameterLinker> synthParamPtr, psParamPtr;

    Project project{ *this, "Untitled" };
    String lastDocumentFilename = "";
    
    OwnedArray<MidiMessageSequence> notesMidiSeq, ccMidiSeq;

    std::atomic<int> ccBase{ 102 }, psModSource{ USE_NONE }, midiChannel{ 0 };

    //==============================================================================
    AudioPlayHead::CurrentPositionInfo posInfo;
    Array<MidiMessage> controllerStateMessage;

    std::atomic<bool> properlyPrepared{ false };

    std::atomic<bool> isPlayingNote{ false }, panicNoteOff{ false };
    std::atomic<int> isWithIn{ -1 };
    bool wasStopped = true;
    std::unordered_map<int, float> playingNotes;

    std::atomic<bool> isHostPlaying{ false };
    std::atomic<int> transportState{ PLUGIN_PAUSED };
    std::atomic<double> transportTimeElasped{ 0.0 };
    CriticalSection transportLock;

    float nextStartTime = -1.0, rangeStartTime = FLT_MAX, rangeEndTime = -FLT_MAX;

    int synthBundleTotalNumInputChannels, synthBundleMainBusNumInputChannels, synthBundleMainBusNumOutputChannels,
        psBundleTotalNumInputChannels, psBundleTotalNumOutputChannels, psBundleMainBusNumInputChannels, psBundleMainBusNumOutputChannels;
    float sampleLength{ -1 }, bufferLength{ -1 };

    bool isCurrentModSrcKontakt = false;
    std::atomic<bool> updateMidSeqSus = false, updateModSrcSus = false, pluginLoadSus = false, loadingDocument = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MicroChromoAudioProcessor)
};
