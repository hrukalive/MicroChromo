/*
  ==============================================================================

    PluginBundle.h
    Created: 8 Mar 2020 9:54:48am
    Author:  bowen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginInstance.h"

class MicroChromoAudioProcessor;

class PluginBundle : ChangeBroadcaster
{
public:
    PluginBundle(size_t numInstances, const PluginDescription desc, MicroChromoAudioProcessor& p);
    ~PluginBundle();

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void releaseResources();
    void processBlock(OwnedArray<AudioBuffer<float>>&, MidiBuffer&);
    int getTotalNumInputChannels() const noexcept { return _numInstances; }
    int getTotalNumOutputChannels() const noexcept { return _numInstances; }

    //==============================================================================
    const String getName() const { return _desc.descriptiveName; }

    //==============================================================================
    void getStateInformation(MemoryBlock& destData);
    void setStateInformation(const void* data, int sizeInBytes);

    //==============================================================================
    void loadPlugin();

    bool isLoading();
    bool isLoaded();

private:
    void addPluginCallback(std::unique_ptr<AudioPluginInstance> instance, const String& error, int index);
    void checkPluginLoaded();

    MicroChromoAudioProcessor& processor;
    AudioPluginFormatManager& formatManager;

    size_t _numInstances = 1;
    const PluginDescription& _desc;
    uint32 uid = 0;
    OwnedArray<PluginInstance> instances;
    CriticalSection criticalSection;
    size_t instanceStarted = 0;
    bool _isLoading = false, _isLoaded = false, _isError = false;
    String errMsg;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginBundle)
};
