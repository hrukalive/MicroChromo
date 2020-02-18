/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginInstance.h"

using GUICallback = std::function<void()>;

//==============================================================================
/**
*/
class MicroChromoAudioProcessor  : public AudioProcessor
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
	void addPlugin(const PluginDescription& desc, bool isSynth, GUICallback callback);
	void addPluginCallback(std::unique_ptr<AudioPluginInstance> instance, const String& error, bool isSynth, int index);

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	//==============================================================================
	ApplicationProperties& getApplicationProperties() { return appProperties; }
	AudioPluginFormatManager& getAudioPluginFormatManager() { return formatManager; }
	KnownPluginList& getKnownPluginList() { return knownPluginList; }
	OwnedArray<PluginInstance>& getSynthArray() { return synthArray; }
	OwnedArray<PluginInstance>& getPitchShiftArray() { return psArray; }
    foleys::LevelMeterSource& getInputMeterSource() { return inputMeterSource; }
	foleys::LevelMeterSource& getOutputMeterSource() { return outputMeterSource; }

private:
    //==============================================================================
	ApplicationProperties appProperties;
	KnownPluginList knownPluginList;
	AudioPluginFormatManager formatManager;
	Array<PluginDescription> internalTypes;

	const int numInstances = 1;
    uint32 uid = 0;
	OwnedArray<PluginInstance> synthArray, psArray;
	OwnedArray<AudioBuffer<float>> bufferArray;

	AudioProcessorValueTreeState parameters;

    foleys::LevelMeterSource inputMeterSource;
	foleys::LevelMeterSource outputMeterSource;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MicroChromoAudioProcessor)
};
