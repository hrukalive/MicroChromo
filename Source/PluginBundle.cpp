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
{
    for (auto i = 0; i < _numInstances; i++)
        collectors.set(i, new MidiMessageCollector());
}

PluginBundle::~PluginBundle()
{
    instances.clear();
    instanceTemps.clear();
    collectors.clear();
}

void PluginBundle::loadPlugin()
{
    if (_isLoading.load())
    {
        AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, "Please Wait", "Wait for current loading to finish", "OK");
        return;
    }
    _isLoading = true;
    _isLoaded = false;
    _isError = false;
    instanceStarted = 0;

    sendSynchronousChangeMessage();
    instanceTemps.clearQuick(false);
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

void PluginBundle::loadPluginSync()
{
    if (_isLoading.load())
    {
        AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, "Please Wait", "Wait for current loading to finish", "OK");
        return;
    }
    _isLoading = true;
    _isLoaded = false;
    _isError = false;
    instanceStarted = 0;

    sendSynchronousChangeMessage();
    instanceTemps.clearQuick(false);
    for (auto i = 0; i < _numInstances; i++)
    {
        String error;
        auto instance = formatManager.createPluginInstance(_desc, processor.getSampleRate(), processor.getBlockSize(), error);
        addPluginCallback(std::move(instance), error, i);
    }
    checkPluginLoaded();
}

PluginInstance* PluginBundle::getInstanceAt(size_t index)
{
    if (index >= instanceStarted.load())
        return nullptr;
    return instances[index];
}

MidiMessageCollector* PluginBundle::getCollectorAt(size_t index)
{
    if (index >= instanceStarted.load())
        return nullptr;
    return collectors[index];
}

void PluginBundle::addPluginCallback(std::unique_ptr<AudioPluginInstance> instance, const String& error, int index)
{
    if (instance == nullptr)
    {
        _isError = true;
        errMsg = error;
    }
    else
    {
        instance->enableAllBuses();
        instanceTemps.set(index, new PluginInstance(uid++, std::move(instance)), true);
        instanceStarted++;
    }
}

void PluginBundle::checkPluginLoaded()
{
    if (instanceStarted.load() == _numInstances)
    {
        _isLoaded = true;
        _isLoading = false;
        if (_isError.load())
        {
            AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon,
                "Loading Failed",
                TRANS("Couldn't create plugin. ") + errMsg,
                "OK");
            _isLoaded = false;
        }
        else
        {
            if (!isInit)
            {
                for (auto* p : instances[0]->processor->getParameters())
                    p->removeListener(this);
            }
            else
                isInit = false;
            for (auto i = 0; i < _numInstances; i++)
                instances.set(i, instanceTemps[i], true);
            instanceTemps.clearQuick(false);
            for (auto* p : instances[0]->processor->getParameters())
                p->addListener(this);
        }
        sendChangeMessage();
    }
}

void PluginBundle::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    _sampleRate = sampleRate;
    _samplesPerBlock = samplesPerBlock;
    for (int i = 0; i < instanceStarted.load(); i++)
    {
        instances[i]->prepare(sampleRate, samplesPerBlock);
        collectors[i]->reset(sampleRate);
    }
}

void PluginBundle::processBlock(OwnedArray<AudioBuffer<float>>& bufferArray, MidiBuffer& midiMessages)
{
    for (auto i = 0; i < instanceStarted.load(); i++)
    {
        collectors[i]->removeNextBlockOfMessages(midiMessages, _samplesPerBlock.load());
        MidiMessage midi_message;
        int sample_offset;

        for (MidiBuffer::Iterator it(midiMessages); it.getNextEvent(midi_message, sample_offset);) {
            if (midi_message.isController()) {
                if (midi_message.getControllerNumber() == 100) {
                    int value = midi_message.getControllerValue();
                    if (value < 128 && hasLearned)
                        instances[i]->processor->getParameters()[learnedCc]->setValueAt(value / 127.0f * (learnedCcMax - learnedCcMin) + learnedCcMin, sample_offset);
                }
            }
        }
        instances[i]->processor->processBlock(*bufferArray[i], midiMessages);
    }
}

void PluginBundle::releaseResources()
{
    for (int i = 0; i < instanceStarted.load(); i++)
        instances[i]->unprepare();
}

bool PluginBundle::isLoading()
{
    return _isLoading.load();
}

bool PluginBundle::isLoaded()
{
    return _isLoaded.load();
}

void PluginBundle::getStateInformation(MemoryBlock& destData)
{
    instances[0]->processor->getStateInformation(destData);
}

void PluginBundle::setStateInformation(const void* data, int sizeInBytes)
{
    for (auto i = 0; i < instanceStarted.load(); i++)
        instances[i]->processor->setStateInformation(data, sizeInBytes);
}

void PluginBundle::propagateState()
{
    MemoryBlock block;
    instances[0]->processor->getStateInformation(block);
    for (auto i = 1; i < instanceStarted.load(); i++)
        instances[i]->processor->setStateInformation(block.getData(), block.getSize());
}

void PluginBundle::parameterValueChanged(int parameterIndex, float newValue)
{
    if (isLearning)
    {
        learnedCc = parameterIndex;
        if (newValue > learnedCcMax)
            learnedCcMax = newValue;
        else if (newValue < learnedCcMin)
            learnedCcMin = newValue;
    }
    for (auto i = 1; i < instanceStarted.load(); i++)
        instances[i]->processor->getParameters()[parameterIndex]->setValue(newValue);
}

void PluginBundle::parameterGestureChanged(int parameterIndex, bool gestureIsStarting)
{
    for (auto i = 1; i < instanceStarted.load(); i++)
    {
        if (gestureIsStarting)
            instances[i]->processor->beginParameterChangeGesture(parameterIndex);
        else
            instances[i]->processor->endParameterChangeGesture(parameterIndex);
    }
}

void PluginBundle::startCcLearn()
{
    hasLearned = false;
    learnedCc = -1;
    learnedCcMin = FP_INFINITE;
    learnedCcMax = -FP_INFINITE;
    isLearning = true;
}

void PluginBundle::stopCcLearn()
{
    if (learnedCc != -1 && learnedCcMin != FP_INFINITE && learnedCcMax != -FP_INFINITE)
        hasLearned = true;
    isLearning = false;
}