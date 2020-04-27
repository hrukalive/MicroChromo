/*
 * This file is part of the MicroChromo distribution
 * (https://github.com/hrukalive/MicroChromo).
 * Copyright (c) 2020 UIUC.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <JuceHeader.h>

//==============================================================================
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

    //==============================================================================
    const uint32 nodeID;
    NamedValueSet properties;
    const std::unique_ptr<AudioProcessor> processor;

private:
    CriticalSection processorLock;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginInstance)
};
