/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
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
	void addPlugin(const PluginDescription& desc, bool isSynth);

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void changeListenerCallback(ChangeBroadcaster* changed) override;

	//==============================================================================
	ApplicationProperties& getApplicationProperties() { return appProperties; }
	AudioPluginFormatManager& getAudioPluginFormatManager() { return formatManager; }
	KnownPluginList& getKnownPluginList() { return knownPluginList; }
    std::shared_ptr<PluginBundle> getSynthBundlePtr() { return synthBundle; }
    std::shared_ptr<PluginBundle> getPSBundlePtr() { return psBundle; }
    foleys::LevelMeterSource& getInputMeterSource() { return inputMeterSource; }
	foleys::LevelMeterSource& getOutputMeterSource() { return outputMeterSource; }
    size_t getNumInstances() { return numInstances; }
    MidiMessageCollector& getMidiMessageCollector() noexcept { return messageCollector; }

private:
    //==============================================================================
	ApplicationProperties appProperties;
	KnownPluginList knownPluginList;
	AudioPluginFormatManager formatManager;
	Array<PluginDescription> internalTypes;

	const size_t numInstances = 1;
	OwnedArray<AudioBuffer<float>> bufferArray;
    std::shared_ptr<PluginBundle> synthBundle, psBundle;

	AudioProcessorValueTreeState parameters;
    MidiMessageCollector messageCollector;

    foleys::LevelMeterSource inputMeterSource;
	foleys::LevelMeterSource outputMeterSource;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MicroChromoAudioProcessor)
};
