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
#include "PluginInstance.h"
#include "PluginBundle.h"
#include "Project.h"
#include "ParameterLinker.h"
#include "Note.h"

//==============================================================================
class MicroChromoAudioProcessor : public AudioProcessor, public ChangeListener, public FileBasedDocument, public HighResolutionTimer
{
public:
    MicroChromoAudioProcessor();
    ~MicroChromoAudioProcessor();

    //===------------------------------------------------------------------===//
    // AudioProcessor
    //===------------------------------------------------------------------===//
#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
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
    void resetState();
    void getStateInformation(MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //===------------------------------------------------------------------===//
    // Internal Playback
    //===------------------------------------------------------------------===//
    void hiResTimerCallback() override;
    void togglePlayback();
    void stopPlayback();
    void setTimeForPlayback(double time);
    int getTransportState();
    double getTimeElapsed();

    //===------------------------------------------------------------------===//
    // Callbacks
    //===------------------------------------------------------------------===//
    void changeListenerCallback(ChangeBroadcaster* changed) override;
    void startLoadingPlugin();
    void finishLoadingPlugin();

    //===------------------------------------------------------------------===//
    // Accessors
    //===------------------------------------------------------------------===//
    ApplicationProperties& getApplicationProperties() { return appProperties; }
    AudioPluginFormatManager& getAudioPluginFormatManager() { return formatManager; }
    UndoManager& getUndoManager() noexcept { return undoManager; }

    KnownPluginList& getKnownPluginList() { return knownPluginList; }
    KnownPluginList& getSynthKnownPluginList() { return synthKnownPluginList; }
    KnownPluginList& getPsKnownPluginList() { return psKnownPluginList; }

    AudioProcessorValueTreeState& getValueTreeState() { return parameters; }
    Project& getProject() { return project; }

    std::shared_ptr<PluginBundle> getSynthBundlePtr() { return synthBundle; }
    std::shared_ptr<PluginBundle> getPSBundlePtr() { return psBundle; }
    int getNumInstances() const noexcept { return numInstancesParameter; }
    int getParameterSlotNumber() const noexcept { return parameterSlotNumber; }
    int getMidiChannel() const noexcept { return midiChannel; }
    int getCcBase() const noexcept { return ccBase; }
    double getMidiSequenceEndTime() const noexcept { return rangeEndTime; }
    int getPitchShiftModulationSource() const noexcept { return psModSource; }

    bool canLearnCc(const PluginBundle* bundle);
    bool canChooseKontakt();
    String getSelectedColorPresetName() const noexcept { return selectedPreset; }
    void setSelectedColorPresetName(String name) { selectedPreset = name; }

    //===------------------------------------------------------------------===//
    // Helpers
    //===------------------------------------------------------------------===//
    void adjustInstanceNumber(int newNumInstances);
    void addPlugin(const PluginDescription& desc, bool isSynth, std::function<void(PluginBundle&)> callback = nullptr);

    void updateMidiSequence(int newBase = -1);
    void updateCcMidiSequenceWithNewBase(int newBase);
    void updateMidiChannel(int newMidiChannel);
    void updateKontaktCcBase(int newCcBase);

    void updatePitchShiftModulationSource(int useKontakt = 0);
    void toggleUseKontakt(bool isOn);

    void triggerPanic() { panicNoteOff = true; }

    static const int MAX_INSTANCES = 8;

private:
    //===------------------------------------------------------------------===//
    // Private Helpers
    //===------------------------------------------------------------------===//
    void addMessageToAllBuffer(OwnedArray<MidiBuffer>& midiBuffers, MidiMessage& msg, int sampleOffset);
    void sendAllNotesOff();
    void sendAllNotesOffPanic();

    //===------------------------------------------------------------------===//
    // FileBasedDocument
    //===------------------------------------------------------------------===//
    String getDocumentTitle() override;
    Result loadDocument(const File& file) override;
    Result saveDocument(const File& file) override;
    File getLastDocumentOpened() override;
    void setLastDocumentOpened(const File& file) override;

    //==============================================================================
    ApplicationProperties appProperties;
    KnownPluginList knownPluginList, synthKnownPluginList, psKnownPluginList;
    AudioPluginFormatManager formatManager;
    Array<PluginDescription> internalTypes;
    AudioProcessorValueTreeState parameters;
    UndoManager undoManager;

    //==============================================================================
    std::atomic<int> numInstancesParameter{ 1 }, parameterSlotNumber{ 16 };
    String selectedPreset = "---INIT---";

    //==============================================================================
    OwnedArray<AudioBuffer<float>> audioBufferArrayA, audioBufferArrayB;
    OwnedArray<MidiBuffer> midiBufferArrayA, midiBufferArrayB;
    std::shared_ptr<PluginBundle> synthBundle, psBundle;

    OwnedArray<ParameterLinker> synthParamPtr, psParamPtr;

    Project project{ *this, "Untitled" };
    String lastDocumentFilename = "";
    
    OwnedArray<MidiMessageSequence> notesMidiSeq, ccMidiSeq;

    std::atomic<int> ccBase{ 102 }, psModSource{ USE_NONE }, midiChannel{ 0 };

    //==============================================================================
    AudioPlayHead::CurrentPositionInfo posInfo;
    Array<MidiMessage> controllerStateMessage;

    std::atomic<bool> properlyPrepared{ false };

    std::atomic<bool> isPlayingNote{ false }, panicNoteOff{ false };
    std::atomic<int> isWithIn{ -1 };
    bool wasStopped = true;
    std::unordered_map<int, float> playingNotes;

    std::atomic<bool> isHostPlaying{ false };
    std::atomic<int> transportState{ PLUGIN_PAUSED };
    std::atomic<double> transportTimeElasped{ 0.0 };
    std::atomic<bool> hasSeeked{ false };
    CriticalSection transportLock;

    float nextStartTime = -1.0, rangeStartTime = FLT_MAX, rangeEndTime = -FLT_MAX;

    int synthBundleTotalNumInputChannels, synthBundleMainBusNumInputChannels, synthBundleMainBusNumOutputChannels,
        psBundleTotalNumInputChannels, psBundleTotalNumOutputChannels, psBundleMainBusNumInputChannels, psBundleMainBusNumOutputChannels;
    float sampleLength{ -1 }, bufferLength{ -1 };

    bool isCurrentModSrcKontakt = false;
    std::atomic<bool> updateMidSeqSus = false, updateModSrcSus = false, pluginLoadSus = false, loadingDocument = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MicroChromoAudioProcessor)
};
