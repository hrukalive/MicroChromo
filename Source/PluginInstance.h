/*
  ==============================================================================

    PluginInstance.h
    Created: 16 Feb 2020 4:04:18pm
    Author:  bowen

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class PluginInstance
{
public:
    PluginInstance(uint32 n, std::shared_ptr<AudioProcessor> p) noexcept
        : nodeID(n), processor(p)
    {
        jassert(processor != nullptr);
    }

    const uint32 nodeID;
    AudioProcessor* getProcessor() const noexcept { return processor.get(); }
    NamedValueSet properties;

    const std::shared_ptr<AudioProcessor> processor;

    void prepare(double newSampleRate, int newBlockSize)
    {
        const ScopedLock lock(processorLock);

        processor->setRateAndBufferSizeDetails(newSampleRate, newBlockSize);
        processor->prepareToPlay(newSampleRate, newBlockSize);
    }
    void unprepare()
    {
        const ScopedLock lock(processorLock);

        processor->releaseResources();
    }

private:
    CriticalSection processorLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginInstance)
};
