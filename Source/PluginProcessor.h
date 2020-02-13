/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

using AudioGraphIOProcessor = AudioProcessorGraph::AudioGraphIOProcessor;
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
	void initializeGraph();
	void connectAudioNodes();
	void connectMidiNodes();
	void updateGraph();
	void addPlugin(const PluginDescription& desc, int slot_number, int copy_number, GUICallback callback);
	void addPluginCallback(std::unique_ptr<AudioPluginInstance> instance, const String& error, int slot_number, int copy_number);

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	//==============================================================================
	ApplicationProperties& getApplicationProperties() { return appProperties; }
	AudioPluginFormatManager& getAudioPluginFormatManager() { return formatManager; }
	KnownPluginList& getKnownPluginList() { return knownPluginList; }
	AudioProcessorGraph& getAudioProcessorGraph() { return mainProcessor; }
    foleys::LevelMeterSource& getMeterSource() { return meterSource; }

private:
    //==============================================================================
	ApplicationProperties appProperties;
	KnownPluginList knownPluginList;
	AudioProcessorGraph mainProcessor;
	AudioPluginFormatManager formatManager;
    AudioProcessorGraph::Node::Ptr audioInputNode;
    AudioProcessorGraph::Node::Ptr audioOutputNode;
    AudioProcessorGraph::Node::Ptr midiInputNode;
    AudioProcessorGraph::Node::Ptr midiOutputNode;

	AudioProcessorValueTreeState parameters;

    foleys::LevelMeterSource meterSource;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MicroChromoAudioProcessor)
};
