/*
  ==============================================================================

    PluginBundle.cpp
    Created: 8 Mar 2020 9:54:48am
    Author:  bowen

  ==============================================================================
*/

#include "PluginBundle.h"
#include "PluginProcessor.h"

PluginBundle::PluginBundle(size_t numInstances, const PluginDescription desc, MicroChromoAudioProcessor& p)
    : _numInstances(numInstances), _desc(desc), processor(p), formatManager(p.getAudioPluginFormatManager())
{}

PluginBundle::~PluginBundle()
{
    instances.clear();
}

void PluginBundle::loadPlugin()
{
    {
        ScopedLock lock(criticalSection);
        if (_isLoaded)
            return;
        if (_isLoading)
        {
            AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, "Please Wait", "Wait for current loading to finish", "OK");
            return;
        }
        _isLoading = true;
        _isLoaded = false;
        _isError = false;
        instanceStarted = 0;
    }
    sendChangeMessage();
    for (auto i = 0; i < _numInstances; i++)
    {
        formatManager.createPluginInstanceAsync(_desc,
            processor.getSampleRate(),
            processor.getBlockSize(),
            [this, i](std::unique_ptr<AudioPluginInstance> instance, const String& error)
            {
                addPluginCallback(std::move(instance), error, i);
                checkPluginLoaded();
            });
    }
}

void PluginBundle::addPluginCallback(std::unique_ptr<AudioPluginInstance> instance, const String& error, int index)
{
    if (instance == nullptr)
    {
        ScopedLock lock(criticalSection);
        _isError = true;
        errMsg = error;
    }
    else
    {
        instance->enableAllBuses();
        instances.set(index, new PluginInstance(uid++, std::move(instance)), true);
        {
            const ScopedLock lock(criticalSection);
            instanceStarted++;
        }
    }
}

void PluginBundle::checkPluginLoaded()
{
    const ScopedLock lock(criticalSection);
    if (instanceStarted == _numInstances)
    {
        _isLoaded = true;
        _isLoading = false;
        if (_isError)
        {
            AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon,
                "Loading Failed",
                TRANS("Couldn't create plugin. ") + errMsg,
                "OK");
            _isLoaded = false;
        }
        sendChangeMessage();
    }
}

void PluginBundle::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    for (int i = 0; i < _numInstances; i++)
        instances[i]->prepare(sampleRate, samplesPerBlock);
}

void PluginBundle::processBlock(OwnedArray<AudioBuffer<float>>& bufferArray, MidiBuffer& midiMessages)
{
    for (auto i = 0; i < instanceStarted; i++)
        instances[i]->processor->processBlock(*bufferArray[i], midiMessages);
}

void PluginBundle::releaseResources()
{
    for (int i = 0; i < _numInstances; i++)
        instances[i]->unprepare();
}

bool PluginBundle::isLoading()
{
    return _isLoading;
}

bool PluginBundle::isLoaded()
{
    return _isLoaded;
}

void PluginBundle::getStateInformation(MemoryBlock& destData)
{
    instances[0]->processor->getStateInformation(destData);
}

void PluginBundle::setStateInformation(const void* data, int sizeInBytes)
{
    instances[0]->processor->setStateInformation(data, sizeInBytes);
}
