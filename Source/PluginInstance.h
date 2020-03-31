/*
  ==============================================================================

    PluginInstance.h
    Created: 16 Feb 2020 4:04:18pm
    Author:  bowen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class PluginInstance
{
public:
    PluginInstance(uint32 n, std::unique_ptr<AudioProcessor> p) noexcept
        : nodeID(n), processor(std::move(p))
    {
        jassert(processor != nullptr);
    }
    ~PluginInstance()
    {
        if (auto* w = processor->getActiveEditor())
            processor->editorBeingDeleted(w);
    }

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

    const uint32 nodeID;
    NamedValueSet properties;
    const std::unique_ptr<AudioProcessor> processor;

private:
    CriticalSection processorLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginInstance)
};
