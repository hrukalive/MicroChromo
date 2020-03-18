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

class PluginBundle : public ChangeBroadcaster, AudioProcessorParameter::Listener
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
    void propagateState();

    //==============================================================================
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override;

    //==============================================================================
    void loadPlugin();
    PluginInstance* getInstanceAt(size_t index);

    bool isLoading();
    bool isLoaded();
    void setPluginDescription(const PluginDescription desc) { _desc = desc; }

private:
    void addPluginCallback(std::unique_ptr<AudioPluginInstance> instance, const String& error, int index);
    void checkPluginLoaded();

    MicroChromoAudioProcessor& processor;
    AudioPluginFormatManager& formatManager;

    size_t _numInstances = 1;
    PluginDescription _desc;
    std::atomic<uint32> uid = 0;
    OwnedArray<PluginInstance> instances;
    std::atomic<int> instanceStarted = 0;
    std::atomic<bool> _isLoading = false, _isLoaded = false, _isError = false;
    String errMsg;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginBundle)
};
