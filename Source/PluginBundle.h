/*
  ==============================================================================

    PluginBundle.h
    Created: 8 Mar 2020 9:54:48am
    Author:  bowen

  ==============================================================================
*/

#pragma once

#include "Common.h"
#include "PluginWindow.h"
#include "PluginInstance.h"
#include "ParameterLinkEditor.h"

class MicroChromoAudioProcessor;

class PluginBundle : public ChangeBroadcaster, AudioProcessorParameter::Listener
{
public:
    PluginBundle(MicroChromoAudioProcessor& p, int maxInstances, OwnedArray<ParameterLinker>& linker, PluginDescription emptyPlugin, PluginDescription defaultPlugin);
    ~PluginBundle();

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void releaseResources();
    void processBlock(OwnedArray<AudioBuffer<float>>&, MidiBuffer&);
    int getTotalNumInputChannels() const noexcept { return instances[0]->processor->getTotalNumInputChannels(); }
    int getTotalNumOutputChannels() const noexcept { return instances[0]->processor->getTotalNumOutputChannels(); }
    int getMainBusNumInputChannels() const noexcept { return instances[0]->processor->getMainBusNumInputChannels(); }
    int getMainBusNumOutputChannels() const noexcept { return instances[0]->processor->getMainBusNumOutputChannels(); }
    MicroChromoAudioProcessor& getProcessor() { return processor; }

    //==============================================================================
    const String getName() const { return currentDesc.descriptiveName; }
    const PluginDescription getDescription() { return currentDesc; }

    //==============================================================================
    void getStateInformation(MemoryBlock& destData);
    void setStateInformation(const void* data, int sizeInBytes);
    void propagateState();

    //==============================================================================
    const Array<AudioProcessorParameter*>& getParameters();
    const Array<int>& getLinkedArray() { return linkParameterIndices; }
    void setParameterValue(int parameterIndex, float newValue);
    void setParameterGesture(int parameterIndex, bool gestureIsStarting);
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override;

    //==============================================================================
    void preLoadPlugin(int numInstances);
    void loadPlugin(const PluginDescription desc, int numInstances, std::function<void(PluginBundle&)> callback = nullptr);
    void loadPluginSync(const PluginDescription desc, int numInstances);
    MidiMessageCollector* getCollectorAt(int index);

    //==============================================================================
    void adjustInstanceNumber(int newNumInstances, std::function<void(void)> callback = nullptr);

    //==============================================================================
    void openParameterLinkEditor();
    void resetParameterLink();
    void linkParameters();
    void linkEditorExit(Array<int> selected);

    bool isLoading() { return _isLoading.load(); }
    bool isLoaded() { return _isLoaded.load(); }
    bool finishedLoading() { return _finishedLoading.load(); }
    bool hasError() { return _isError.load(); }
    void resetCcLearn();
    void startCcLearn();
    void stopCcLearn();
    void setCcLearn(int index, float min, float max);
    int getLearnedCc() { return learnedCc; }
    PluginDescription getEmptyPluginDescription() { return _emptyPlugin; }
    PluginDescription getDefaultPluginDescription() { return _defaultPlugin; }

    //==============================================================================
    void closeAllWindows();
    void showRepresentativeWindow();
    void showTwoWindows();
    void showAllWindows();
    void showWindow(PluginWindow::Type type, int num = 1);
    std::unique_ptr<PopupMenu> getPopupMenu(KnownPluginList::SortMethod pluginSortMethod, KnownPluginList& knownPluginList);

private:
    void addPluginCallback(std::unique_ptr<AudioPluginInstance> instance, const String& error, int index);
    void checkPluginLoaded(const PluginDescription desc, int numInstances, std::function<void(PluginBundle&)> callback = nullptr);

    MicroChromoAudioProcessor& processor;
    AudioPluginFormatManager& formatManager;
    std::atomic<double> _sampleRate;
    std::atomic<int> _samplesPerBlock;

    int _numInstances = 1, _maxInstances = 8;
    PluginDescription currentDesc;
    std::atomic<uint32> uid = 0;
    OwnedArray<PluginInstance> instances;
    OwnedArray<PluginInstance> instanceTemps;
    const OwnedArray<ParameterLinker>& parameterLinker;
    Array<int> linkParameterIndices;
    OwnedArray<MidiMessageCollector> collectors;
    PluginDescription _emptyPlugin, _defaultPlugin;
    std::atomic<int> instanceStarted = 0;
    std::atomic<int> instanceStartedTemp = 0;
    std::atomic<bool> _isLoading = false, _isLoaded = false, _finishedLoading = false, _isError = false, isInit = true, isLearning = false, hasLearned = false;
    std::atomic<int> learnedCc{ -1 };
    std::atomic<float> learnedCcMin{ FP_INFINITE }, learnedCcMax{ -FP_INFINITE };
    String errMsg;

    OwnedArray<PluginWindow> activePluginWindows;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginBundle)
};
