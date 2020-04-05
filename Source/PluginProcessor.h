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

    void updateMidiSequence(MidiMessageSequence seq);
    void sendAllNotesOff(MidiBuffer& midiMessages);

    //==============================================================================
    ApplicationProperties& getApplicationProperties() { return appProperties; }
    AudioPluginFormatManager& getAudioPluginFormatManager() { return formatManager; }
    KnownPluginList& getKnownPluginList() { return knownPluginList; }
    KnownPluginList& getSynthKnownPluginList() { return synthKnownPluginList; }
    KnownPluginList& getPsKnownPluginList() { return psKnownPluginList; }
    std::shared_ptr<PluginBundle> getSynthBundlePtr() { return synthBundle; }
    std::shared_ptr<PluginBundle> getPSBundlePtr() { return psBundle; }
    //foleys::LevelMeterSource& getInputMeterSource() { return inputMeterSource; }
    //foleys::LevelMeterSource& getOutputMeterSource() { return outputMeterSource; }
    int getNumInstances() { return numInstancesParameter; }
    AudioProcessorValueTreeState& getValueTreeState() { return parameters; }
    UndoManager* getUndoManager() noexcept { return &undoManager; }
    OwnedArray<ParameterLinker>& getSynthParameterLinker() { return synthParamPtr; }
    OwnedArray<ParameterLinker>& getPSParameterLinker() { return psParamPtr; }
    int getParameterSlotNumber() { return parameterSlotNumber; }

    static const int MAX_INSTANCES = 8;
private:
    //==============================================================================
    ApplicationProperties appProperties;
    KnownPluginList knownPluginList, synthKnownPluginList, psKnownPluginList;
    AudioPluginFormatManager formatManager;
    Array<PluginDescription> internalTypes;

    AudioPlayHead::CurrentPositionInfo posInfo;
    int parameterSlotNumber = 16;
    std::atomic<bool> isPlayingNote{ false };

    std::atomic<int> numInstancesParameter{ 1 };
    int synthBundleTotalNumInputChannels, synthBundleMainBusNumInputChannels, synthBundleMainBusNumOutputChannels,
        psBundleTotalNumInputChannels, psBundleTotalNumOutputChannels, psBundleMainBusNumInputChannels, psBundleMainBusNumOutputChannels;
    std::atomic<bool> properlyPrepared{ false };
    OwnedArray<AudioBuffer<float>> bufferArrayA, bufferArrayB;
    std::shared_ptr<PluginBundle> synthBundle, psBundle;

    UndoManager undoManager;
    AudioProcessorValueTreeState parameters;
    OwnedArray<ParameterLinker> synthParamPtr, psParamPtr;

    MidiMessageSequence midiSeq;
    double nextStartTime = -1.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MicroChromoAudioProcessor)
};
