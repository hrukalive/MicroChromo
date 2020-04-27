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

#include "Common.h"
#include "PluginWindow.h"

class MicroChromoAudioProcessor;
class PluginInstance;
class ParameterLinker;
class ParameterLinkEditor;
class ParameterCcLearn;

//==============================================================================
class PluginBundle : public ChangeBroadcaster, public AudioProcessorParameter::Listener
{
public:
    PluginBundle(MicroChromoAudioProcessor& p, 
        int maxInstances, 
        OwnedArray<ParameterLinker>& linker, 
        PluginDescription emptyPlugin, 
        PluginDescription defaultPlugin);
    ~PluginBundle();

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void releaseResources();
    void processBlock(OwnedArray<AudioBuffer<float>>&, OwnedArray<MidiBuffer>&);

    //===------------------------------------------------------------------===//
    // Accessors
    //===------------------------------------------------------------------===//
    MicroChromoAudioProcessor& getProcessor() { return processor; }
    const String getName() const { return currentDesc.descriptiveName; }
    const PluginDescription getDescription() { return currentDesc; }
    PluginDescription getEmptyPluginDescription() { return _emptyPlugin; }
    PluginDescription getDefaultPluginDescription() { return _defaultPlugin; }
    int getTotalNumInputChannels() const noexcept;
    int getTotalNumOutputChannels() const noexcept;
    int getMainBusNumInputChannels() const noexcept;
    int getMainBusNumOutputChannels() const noexcept;
    bool isKontakt();

    PluginInstance* getMainInstance() { return instances[0]; }
    const Array<AudioProcessorParameter*>& getParameters(int index = 0);
    ParameterCcLearn& getCcLearnModule() { return *ccLearn; }

    const Array<int>& getLinkedArray() { return linkParameterIndices; }
    bool isParameterExposed(int parameterIndex) { return linkParameterIndicesSet.contains(parameterIndex); }

    bool isLoading() { return _isLoading.load(); }
    bool isLoaded() { return _isLoaded.load(); }
    bool isNewlyLoaded() { return _isNewlyLoaded.load(); }
    bool finishedLoading() { return _finishedLoading.load(); }
    bool hasError() { return _isError.load(); }

    //===------------------------------------------------------------------===//
    // Serialization
    //===------------------------------------------------------------------===//
    std::unique_ptr<XmlElement> createXml(String rootTag);
    void loadFromXml(const XmlElement* xml, String rootTag);

    //===------------------------------------------------------------------===//
    // Listeners
    //===------------------------------------------------------------------===//
    void setParameterValue(int parameterIndex, float newValue);
    void setParameterGesture(int parameterIndex, bool gestureIsStarting);
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override;

    //===------------------------------------------------------------------===//
    // Helpers
    //===------------------------------------------------------------------===//
    void propagateState();

    void preLoadPlugin(int numInstances);
    void loadPlugin(const PluginDescription desc, int numInstances, std::function<void(PluginBundle&)> callback = nullptr);
    void loadPluginSync(const PluginDescription desc, int numInstances);

    void adjustInstanceNumber(int newNumInstances, std::function<void(void)> callback = nullptr);

    void resetParameterLink();
    void linkParameters();

    //===------------------------------------------------------------------===//
    // UI Helpers
    //===------------------------------------------------------------------===//
    void closeAllWindows();
    void showWindow(int num = 1, PluginWindow::Type type = PluginWindow::Type::normal);
    void bringToFront();

    void openParameterLinkEditor();

    std::unique_ptr<PopupMenu> getMainPopupMenu();
    std::unique_ptr<PopupMenu> getPluginPopupMenu(KnownPluginList::SortMethod pluginSortMethod, KnownPluginList& knownPluginList);

private:
    friend class ParameterLinkEditor;

    //===------------------------------------------------------------------===//
    // Private Helpers
    //===------------------------------------------------------------------===//
    void addPluginCallback(std::unique_ptr<AudioPluginInstance> instance, const String& error, int index);
    void checkPluginLoaded(const PluginDescription desc, int numInstances, std::function<void(PluginBundle&)> callback = nullptr);
    void linkEditorExit(Array<int> selected);

    //==============================================================================
    MicroChromoAudioProcessor& processor;
    AudioPluginFormatManager& formatManager;
    std::atomic<double> _sampleRate{ -1 };
    std::atomic<int> _samplesPerBlock;

    //==============================================================================
    int _numInstances = 1, _maxInstances = 8;
    PluginDescription currentDesc;
    std::atomic<int> uid{ 0 };
    OwnedArray<PluginInstance> instances;
    OwnedArray<PluginInstance> instanceTemps;
    std::atomic<int> instanceStarted{ 0 }, instanceStartedTemp{ 0 };
    std::atomic<bool> _isLoading = false, _isLoaded = false, _finishedLoading = false, 
        _isNewlyLoaded = false, _isError = false, isInit = true;
    String errMsg;

    //==============================================================================
    const OwnedArray<ParameterLinker>& parameterLinker;
    std::unique_ptr<ParameterCcLearn> ccLearn{ nullptr };
    Array<int> linkParameterIndices;
    std::unordered_set<int> linkParameterIndicesSet;
    std::unordered_map<int, int> linkParameterIndicesHistory;
    PluginDescription _emptyPlugin, _defaultPlugin;

    OwnedArray<PluginWindow> activePluginWindows;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginBundle)
};
