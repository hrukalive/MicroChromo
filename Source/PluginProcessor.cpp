/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "InternalPlugins.h"

#include "Note.h"

//==============================================================================
MicroChromoAudioProcessor::MicroChromoAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
                       .withInput("Aux In 1", AudioChannelSet::stereo(), true)
                       .withInput("Aux In 2", AudioChannelSet::stereo(), true)
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                       ),
      FileBasedDocument("mcmproj", "*.mcmproj", "Choose a project to open..", "Choose a file to save as.."),
      parameters(*this, nullptr)
#endif
{
    PropertiesFile::Options options;
    options.folderName = "MicroChromo";
    options.applicationName = "MicroChromo";
    options.filenameSuffix = "usersettings";
    options.osxLibrarySubFolder = "Application Support";
#if JUCE_LINUX
    options.folderName = "~/.config";
#else
    options.folderName = "";
#endif
    appProperties.setStorageParameters(options);

    enableAllBuses();

    knownPluginList.addChangeListener(this);
    formatManager.addDefaultFormats();
    formatManager.addFormat(new InternalPluginFormat());
    InternalPluginFormat internalFormat;
    internalFormat.getAllTypes(internalTypes);
    if (auto savedPluginList = appProperties.getUserSettings()->getXmlValue("pluginList"))
        knownPluginList.recreateFromXml(*savedPluginList);
    parameterSlotNumber = jmax(appProperties.getUserSettings()->getIntValue("parameterSlotNumber", 16), 4);

    for (int i = 0; i < MAX_INSTANCES; i++)
    {
        notesMidiSeq.add(new MidiMessageSequence());
        ccMidiSeq.add(new MidiMessageSequence());
    }

    std::unique_ptr<AudioProcessorParameterGroup> synthParamGroup = std::make_unique<AudioProcessorParameterGroup>("synth_param_group", "Synth Parameters", "-");
    std::unique_ptr<AudioProcessorParameterGroup> psParamGroup = std::make_unique<AudioProcessorParameterGroup>("ps_param_group", "PitchShift Parameters", "-");
    for (auto i = 0; i < 16; i++)
    {
        std::unique_ptr<AudioParameterFloatVariant> param = std::make_unique<AudioParameterFloatVariant>("synth_" + String(i), "Synth Param " + String(i), 0, 1, 0);
        synthParamPtr.add(new ParameterLinker(param.get(), synthBundle));
        synthParamGroup->addChild(std::move(param));
    }
    for (auto i = 0; i < 16; i++)
    {
        std::unique_ptr<AudioParameterFloatVariant> param = std::make_unique<AudioParameterFloatVariant>("ps_" + String(i), "PS Param " + String(i), 0, 1, 0);
        psParamPtr.add(new ParameterLinker(param.get(), psBundle));
        psParamGroup->addChild(std::move(param));
    }

    addParameterGroup(std::move(synthParamGroup));
    addParameterGroup(std::move(psParamGroup));

    synthBundle.reset(new PluginBundle(*this, MAX_INSTANCES, synthParamPtr, internalTypes[0], internalTypes[1]));
    psBundle.reset(new PluginBundle(*this, MAX_INSTANCES, psParamPtr, internalTypes[0], internalTypes[2]));

    synthBundle->loadPluginSync(internalTypes[0], numInstancesParameter);
    psBundle->loadPluginSync(internalTypes[0], numInstancesParameter);

    controllerStateMessage.ensureStorageAllocated(128);

    startTimer(10);
}

MicroChromoAudioProcessor::~MicroChromoAudioProcessor()
{
    stopTimer();
    knownPluginList.removeChangeListener(this);
    synthParamPtr.clear();
    psParamPtr.clear();
    synthBundle = nullptr;
    psBundle = nullptr;
    audioBufferArrayA.clear();
    audioBufferArrayB.clear();
    midiBufferArrayA.clear();
    midiBufferArrayB.clear();
    notesMidiSeq.clear();
    ccMidiSeq.clear();
}

//==============================================================================
const String MicroChromoAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MicroChromoAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MicroChromoAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MicroChromoAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MicroChromoAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MicroChromoAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MicroChromoAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MicroChromoAudioProcessor::setCurrentProgram (int /*index*/)
{
}

const String MicroChromoAudioProcessor::getProgramName (int /*index*/)
{
    return {};
}

void MicroChromoAudioProcessor::changeProgramName (int /*index*/, const String& /*newName*/)
{
}

//==============================================================================
void MicroChromoAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sampleLength = 1.0 / sampleRate;
    bufferLength = samplesPerBlock / sampleRate;

    playingNotes.clear();

    audioBufferArrayA.clear();
    audioBufferArrayB.clear();
    midiBufferArrayA.clear();
    midiBufferArrayB.clear();

    synthBundleTotalNumInputChannels = synthBundle->getTotalNumInputChannels();
    synthBundleMainBusNumInputChannels = synthBundle->getMainBusNumInputChannels();
    synthBundleMainBusNumOutputChannels = synthBundle->getMainBusNumOutputChannels();
    psBundleTotalNumInputChannels = psBundle->getTotalNumInputChannels();
    psBundleTotalNumOutputChannels = psBundle->getTotalNumOutputChannels();
    psBundleMainBusNumInputChannels = psBundle->getMainBusNumInputChannels();
    psBundleMainBusNumOutputChannels = psBundle->getMainBusNumOutputChannels();
    int aChannelNum = jmax(synthBundleTotalNumInputChannels, synthBundle->getTotalNumOutputChannels());
    int bChannelNum = jmax(psBundleTotalNumInputChannels, psBundle->getTotalNumOutputChannels());
    for (int i = 0; i < numInstancesParameter; i++)
    {
        audioBufferArrayA.add(new AudioBuffer<float>(aChannelNum, samplesPerBlock));
        audioBufferArrayB.add(new AudioBuffer<float>(bChannelNum, samplesPerBlock));
        midiBufferArrayA.add(new MidiBuffer());
        midiBufferArrayB.add(new MidiBuffer());
    }

    synthBundle->prepareToPlay(sampleRate, samplesPerBlock);
    psBundle->prepareToPlay(sampleRate, samplesPerBlock);

    properlyPrepared = true;
}

void MicroChromoAudioProcessor::releaseResources()
{
    properlyPrepared = false;

    synthBundle->releaseResources();
    psBundle->releaseResources();
    audioBufferArrayA.clear();
    audioBufferArrayB.clear();
    midiBufferArrayA.clear();
    midiBufferArrayB.clear();
}

void MicroChromoAudioProcessor::addMessageToAllBuffer(OwnedArray<MidiBuffer>& midiBuffers, MidiMessage& msg, int sampleOffset)
{
    for (auto* midiBuffer : midiBuffers)
        midiBuffer->addEvent(msg, sampleOffset);
}

void MicroChromoAudioProcessor::sendAllNotesOff()
{
    for (auto* midiBuffer : midiBufferArrayA)
    {
        midiBuffer->clear();
        for (auto j = 1; j <= 16; j++)
        {
            midiBuffer->addEvent(MidiMessage::allNotesOff(j), 16);
            midiBuffer->addEvent(MidiMessage::allSoundOff(j), 16);
            midiBuffer->addEvent(MidiMessage::allControllersOff(j), 16);
            for (auto& n : playingNotes)
                midiBuffer->addEvent(MidiMessage::noteOff(j, n.first), 18);
            playingNotes.clear();
        }
    }
    for (auto* midiBuffer : midiBufferArrayB)
    {
        midiBuffer->clear();
        for (auto j = 1; j <= 16; j++)
        {
            midiBuffer->addEvent(MidiMessage::allNotesOff(j), 16);
            midiBuffer->addEvent(MidiMessage::allSoundOff(j), 16);
            midiBuffer->addEvent(MidiMessage::allControllersOff(j), 16);
        }
    }
    DBG("All note off");

    isPlayingNote = false;
}

void MicroChromoAudioProcessor::sendAllNotesOffPanic()
{
    for (auto* midiBuffer : midiBufferArrayA)
    {
        midiBuffer->clear();
        for (auto j = 1; j <= 16; j++)
        {
            midiBuffer->addEvent(MidiMessage::allNotesOff(j), 18);
            midiBuffer->addEvent(MidiMessage::allSoundOff(j), 18);
            midiBuffer->addEvent(MidiMessage::allControllersOff(j), 18);
            for (int k = 0; k < 128; k++)
                midiBuffer->addEvent(MidiMessage::noteOff(j, k), 18);
            playingNotes.clear();
        }
    }
    for (auto* midiBuffer : midiBufferArrayB)
    {
        midiBuffer->clear();
        for (auto j = 1; j <= 16; j++)
        {
            midiBuffer->addEvent(MidiMessage::allNotesOff(j), 16);
            midiBuffer->addEvent(MidiMessage::allSoundOff(j), 16);
            midiBuffer->addEvent(MidiMessage::allControllersOff(j), 16);
            for (int k = 0; k < 128; k++)
                midiBuffer->addEvent(MidiMessage::noteOff(j, k), 18);
        }
    }
    DBG("All note off (panic)");

    isPlayingNote = false;
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MicroChromoAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != AudioChannelSet::disabled()
        && layouts.getMainInputChannelSet() != AudioChannelSet::stereo())
        return false;
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;
    return true;
}
#endif

void MicroChromoAudioProcessor::hiResTimerCallback()
{
    if (!isHostPlaying)
    {
        ScopedLock lock(transportLock);
        switch (transportState)
        {
        case HOST_PLAYING: transportState = PLUGIN_PAUSED; break;
        case PLUGIN_PLAYING: transportTimeElasped += getTimerInterval() / 1000.0; break;
        case PLUGIN_QUERY_PLAY: transportState = PLUGIN_PLAYING; break;
        case PLUGIN_QUERY_PAUSE: transportState = PLUGIN_PAUSED; break;
        case PLUGIN_QUERY_STOP: transportTimeElasped = 0; transportState = PLUGIN_STOPPED; break;
        default:
            break;
        }
        if (transportTimeElasped > rangeEndTime)
            transportState = PLUGIN_QUERY_STOP;
    }
    else
        transportState = HOST_PLAYING;
}

void MicroChromoAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;

    posInfo.isPlaying = false;
    isHostPlaying = false;

    auto startTime = 0.0;

    auto* playhead = getPlayHead();
    if (playhead != nullptr)
    {
        playhead->getCurrentPosition(posInfo);

        if (posInfo.isPlaying)
        {
            isHostPlaying = true;
            transportTimeElasped = posInfo.timeInSeconds;
            startTime = posInfo.timeInSeconds;
        }
    }

    if (!posInfo.isPlaying)
    {
        startTime = transportTimeElasped.load();
        if (transportState == PLUGIN_PLAYING)
            posInfo.isPlaying = true;
    }

    auto endTime = startTime + bufferLength;
    auto isWithInNow = (rangeStartTime < rangeEndTime && startTime >= rangeStartTime && startTime < rangeEndTime) ? 1 : 0;

    auto forwardMidiBuffer = [&]()
    {
        MidiMessage midiMessage;
        int sampleOffset;
        for (MidiBuffer::Iterator it(midiMessages); it.getNextEvent(midiMessage, sampleOffset);)
        {
            midiBufferArrayA[0]->addEvent(midiMessage, sampleOffset);
            midiBufferArrayB[0]->addEvent(midiMessage, sampleOffset);
        }
    };

    auto& ccMidiBufferArray = (psModSource == USE_NONE || psModSource == USE_PS) ? midiBufferArrayB : midiBufferArrayA;

    auto ensureCcValue = [&]()
    {
        for (int i = 0; i < numInstancesParameter; i++)
        {
            for (int k = (midiChannel == 0 ? 1 : midiChannel); k <= (midiChannel == 0 ? 16 : midiChannel); k++)
            {
                controllerStateMessage.clear();
                ccMidiSeq[i]->createControllerUpdatesForTime(k, startTime, controllerStateMessage);
                for (auto& msg : controllerStateMessage)
                {
                    ccMidiBufferArray[i]->addEvent(msg, 8);
                    DBG("[PROC_BLOCK] Recovering CC: " << msg.getControllerNumber() << ": " << msg.getControllerValue() << " Chn: " << msg.getChannel());
                }
            }
        }
    };

    // MIDI processing
    MidiBuffer midiBuffer;
    if (!posInfo.isPlaying)
    {
        wasStopped = true;
        if (isPlayingNote)
        {
            midiMessages.clear();
            sendAllNotesOff();
            DBG("[PROC_BLOCK] Playing just now, and now stopped");
        }
        else
        {
            forwardMidiBuffer();
            midiMessages.clear();
        }
    }
    else
    {
        bool noteOffSent = false;
        //DBG("[PROC_BLOCK] " << startTime << " " << endTime << " | " << nextStartTime);
        if (nextStartTime > 0.0 && std::abs(startTime - nextStartTime) > bufferLength)
        {
            if (!noteOffSent)
            {
                sendAllNotesOff();
                ensureCcValue();
                noteOffSent = true;
            }
            DBG("[PROC_BLOCK] Playhead moved by cursor or looping");
        }
        nextStartTime = endTime;

        if (isWithIn == -1 || isWithIn != isWithInNow)
        {
            if (!noteOffSent)
            {
                sendAllNotesOff();
                ensureCcValue();
                noteOffSent = true;
            }
            isWithIn = isWithInNow;
            DBG("[PROC_BLOCK] Playing range entered or exited.");
        }

        if (wasStopped)
        {
            if (!noteOffSent)
            {
                sendAllNotesOff();
                ensureCcValue();
                noteOffSent = true;
            }
            wasStopped = false;
            DBG("[PROC_BLOCK] Stopped before, now playing.");
        }

        if (!noteOffSent) // If all note off sent, then skip this block
        {
            if (isWithInNow == 1)
            {
                // Foward other MIDI messages like controllers
                MidiMessage midiMessage;
                int sampleOffset;
                for (MidiBuffer::Iterator it(midiMessages); it.getNextEvent(midiMessage, sampleOffset);)
                {
                    if (!midiMessage.isNoteOnOrOff())
                    {
                        if (psModSource == USE_PS)
                        {
                            addMessageToAllBuffer(midiBufferArrayA, midiMessage, sampleOffset);
                            if (!midiMessage.isControllerOfType(psBundle->getCcLearnModule().getCcSource()))
                                addMessageToAllBuffer(midiBufferArrayB, midiMessage, sampleOffset);
                        }
                        else if (psModSource == USE_SYNTH)
                        {
                            if (!midiMessage.isControllerOfType(synthBundle->getCcLearnModule().getCcSource()))
                                addMessageToAllBuffer(midiBufferArrayA, midiMessage, sampleOffset);
                            addMessageToAllBuffer(midiBufferArrayB, midiMessage, sampleOffset);
                        }
                        else if (psModSource == USE_KONTAKT)
                        {
                            if (!midiMessage.isController() ||
                                midiMessage.getControllerNumber() < ccBase ||
                                midiMessage.getControllerNumber() >= ccBase + 12)
                                addMessageToAllBuffer(midiBufferArrayA, midiMessage, sampleOffset);
                            addMessageToAllBuffer(midiBufferArrayB, midiMessage, sampleOffset);
                        }
                    }
                }

                // Play note prior to start time that are not yet off.
                if (!isPlayingNote)
                {
                    for (int i = 0; i < numInstancesParameter; i++)
                    {
                        const auto noteEvtIndex = notesMidiSeq[i]->getNextIndexAtTime(startTime);
                        for (int j = 0; j < noteEvtIndex; j++)
                        {
                            MidiMessageSequence::MidiEventHolder* evt = notesMidiSeq[i]->getEventPointer(j);
                            if (evt->message.isNoteOn() && evt->noteOffObject != nullptr && evt->noteOffObject->message.getTimeStamp() > startTime)
                            {
                                midiBufferArrayA[i]->addEvent(evt->message, 2);
                                playingNotes[evt->message.getNoteNumber()] = evt->message.getTimeStamp();
                                DBG("[PROC_BLOCK] Retrigger note on " << evt->message.getNoteNumber() << " at " << evt->message.getTimeStamp());
                                isPlayingNote = true;
                            }
                        }
                    }
                }
                // Stop note prior to start time that are not yet off.
                if (isPlayingNote)
                {
                    for (int i = 0; i < numInstancesParameter; i++)
                    {
                        const auto noteEvtIndex = notesMidiSeq[i]->getNextIndexAtTime(startTime);
                        for (int j = 0; j < noteEvtIndex; j++)
                        {
                            MidiMessageSequence::MidiEventHolder* evt = notesMidiSeq[i]->getEventPointer(j);
                            if (evt->message.isNoteOff() && 
                                playingNotes.find(evt->message.getNoteNumber()) != playingNotes.end() && 
                                playingNotes[evt->message.getNoteNumber()] < evt->message.getTimeStamp())
                            {
                                midiBufferArrayA[i]->addEvent(evt->message, 16);
                                playingNotes.erase(evt->message.getNoteNumber());
                                DBG("[PROC_BLOCK] Retrigger note off " << evt->message.getNoteNumber() << " at " << evt->message.getTimeStamp());
                            }
                        }
                    }
                    isPlayingNote = !playingNotes.empty();
                }

                // Move messages in sequences to buffer for playback.
                for (int i = 0; i < numInstancesParameter; i++)
                {
                    for (int j = notesMidiSeq[i]->getNextIndexAtTime(startTime); j < notesMidiSeq[i]->getNumEvents(); j++)
                    {
                        MidiMessageSequence::MidiEventHolder* evt = notesMidiSeq[i]->getEventPointer(j);

                        if (evt->message.getTimeStamp() >= startTime && evt->message.getTimeStamp() < endTime)
                        {
                            midiBufferArrayA[i]->addEvent(evt->message, roundDoubleToInt((evt->message.getTimeStamp() - startTime) * sampleLength));
                            if (evt->message.isNoteOn())
                            {
                                playingNotes[evt->message.getNoteNumber()] = evt->message.getTimeStamp();
                                DBG("[PROC_BLOCK] Play note: " << evt->message.getNoteNumber() << " at " << evt->message.getTimeStamp());
                            }
                            else if (evt->message.isNoteOff())
                            {
                                playingNotes.erase(evt->message.getNoteNumber());
                                DBG("[PROC_BLOCK] Stop note: " << evt->message.getNoteNumber() << " at " << evt->message.getTimeStamp());
                            }
                            isPlayingNote = true;
                        }
                        if (evt->message.getTimeStamp() >= endTime)
                            break;
                    }
                    for (int j = ccMidiSeq[i]->getNextIndexAtTime(startTime); j < ccMidiSeq[i]->getNumEvents(); j++)
                    {
                        MidiMessageSequence::MidiEventHolder* evt = ccMidiSeq[i]->getEventPointer(j);

                        if (evt->message.getTimeStamp() >= startTime && evt->message.getTimeStamp() < endTime && 
                            (midiChannel == 0 || evt->message.isForChannel(midiChannel)))
                        {
                            ccMidiBufferArray[i]->addEvent(evt->message, roundDoubleToInt((evt->message.getTimeStamp() - startTime) * sampleLength));
                            DBG("[PROC_BLOCK] Adjust CC: " << evt->message.getNoteNumber() << " at " << evt->message.getTimeStamp());
                        }
                        if (evt->message.getTimeStamp() >= endTime)
                            break;
                    }
                }

            }
            else
            {
                forwardMidiBuffer();
                midiMessages.clear();
            }
        }
    }

    if (panicNoteOff)
    {
        panicNoteOff = false;
        sendAllNotesOffPanic();
    }

    // Synthesis
    for (auto i = 0; i < numInstancesParameter; i++)
    {
        int splitPoint = jmin(synthBundleMainBusNumInputChannels, 2, buffer.getNumChannels());
        for (int j = 0; j < splitPoint; j++)
            audioBufferArrayA[i]->copyFrom(j, 0, buffer, j, 0, buffer.getNumSamples());
        for (int j = splitPoint; j < synthBundleTotalNumInputChannels; j++)
            audioBufferArrayA[i]->clear(j, 0, audioBufferArrayA[i]->getNumSamples());
    }
    synthBundle->processBlock(audioBufferArrayA, midiBufferArrayA);

    for (auto i = 0; i < numInstancesParameter; i++)
    {
        int splitPoint = jmin(2 + psBundleMainBusNumInputChannels, psBundleTotalNumInputChannels, buffer.getNumChannels());
        for (int j = 0; j < jmin(synthBundleMainBusNumOutputChannels, psBundleMainBusNumInputChannels); j++)
            audioBufferArrayB[i]->copyFrom(j, 0, *audioBufferArrayA[i], j, 0, audioBufferArrayA[i]->getNumSamples());
        for (int j = psBundleMainBusNumInputChannels; j < splitPoint; j++)
            audioBufferArrayB[i]->copyFrom(j, 0, buffer, j - psBundleMainBusNumInputChannels + 2, 0, buffer.getNumSamples());
        for (int j = splitPoint; j < psBundleTotalNumInputChannels; j++)
            audioBufferArrayB[i]->clear(j, 0, audioBufferArrayB[i]->getNumSamples());
    }
    psBundle->processBlock(audioBufferArrayB, midiBufferArrayB);

    for (auto i = 0; i < buffer.getNumChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
    for (auto i = 0; i < numInstancesParameter; i++)
        for (auto j = 0; j < jmin(getMainBusNumOutputChannels(), psBundleMainBusNumOutputChannels); j++)
            buffer.addFrom(j, 0, *audioBufferArrayB[i], j, 0, buffer.getNumSamples());

    for (auto i = getMainBusNumOutputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
}

void MicroChromoAudioProcessor::adjustInstanceNumber(int newNumInstances)
{
    psBundle->adjustInstanceNumber(newNumInstances);
    synthBundle->adjustInstanceNumber(newNumInstances, [newNumInstances, this]()
        {
            this->numInstancesParameter = newNumInstances;
            updateMidiSequence();
        });
}

void MicroChromoAudioProcessor::updateMidiSequence(int newBase)
{
    updateMidSeqSus = true;
    suspendProcessing(pluginLoadSus || updateModSrcSus || updateMidSeqSus || loadingDocument);

    if (newBase != -1 && ccBase != newBase)
        ccBase = newBase;

    std::unordered_set<int> channels;
    for (auto* track : project.getNoteTracks())
        channels.insert(track->getTrackChannel());

    notesMidiSeq.clear();
    ccMidiSeq.clear();
    for (int i = 0; i < numInstancesParameter; i++)
    {
        notesMidiSeq.add(new MidiMessageSequence());
        ccMidiSeq.add(new MidiMessageSequence());
        for (int j = 0; j < (psModSource == USE_KONTAKT ? 12 : 1); j++)
            for (auto k : channels)
                ccMidiSeq[i]->addEvent(MidiMessage::controllerEvent(k, ccBase + j, 50));
    }

    project.getScheduler()->schedule(notesMidiSeq, ccMidiSeq, numInstancesParameter, ccBase, psModSource == USE_KONTAKT, jmin(2048 * sampleLength, 2 * bufferLength));

    if (psModSource == USE_KONTAKT)
        isCurrentModSrcKontakt = true;
    else
        isCurrentModSrcKontakt = false;

    for (int i = 0; i < notesMidiSeq.size(); i++)
        for (int j = 0; j < (psModSource == USE_KONTAKT ? 12 : 1); j++)
            for (auto k : channels)
                ccMidiSeq[i]->addEvent(MidiMessage::controllerEvent(k, ccBase + j, 50).withTimeStamp(notesMidiSeq[i]->getEndTime() + 5));

    rangeStartTime = FLT_MAX;
    rangeEndTime = -FLT_MAX;
    for (auto& seq : notesMidiSeq)
    {
        if (rangeStartTime > seq->getStartTime())
            rangeStartTime = seq->getStartTime();
        if (rangeEndTime < seq->getEndTime())
            rangeEndTime = seq->getEndTime();
    }
    rangeEndTime += 1;

    updateMidSeqSus = false;
    suspendProcessing(pluginLoadSus || updateModSrcSus || updateMidSeqSus || loadingDocument);
}

void MicroChromoAudioProcessor::updateCcMidiSequenceWithNewBase(int newBase)
{
    if (ccBase == newBase)
        return;

    for (int i = 0; i < numInstancesParameter; i++)
    {
        auto* oldCcSeq = ccMidiSeq[i];
        auto* newSeq = new MidiMessageSequence();
        for (auto& cc : *oldCcSeq)
        {
            MidiMessage msg(MidiMessage::controllerEvent(cc->message.getChannel(), cc->message.getControllerNumber() - ccBase + newBase, cc->message.getControllerValue()));
            msg.setTimeStamp(cc->message.getTimeStamp());
            newSeq->addEvent(msg);
        }
        ccMidiSeq.set(i, newSeq, true);
    }
    ccBase = newBase;
    DBG("CC base updated to " << newBase);
}

//==============================================================================
bool MicroChromoAudioProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* MicroChromoAudioProcessor::createEditor()
{
    return new MicroChromoAudioProcessorEditor (*this);
}

//==============================================================================
void MicroChromoAudioProcessor::reset()
{
    undoManager.clearUndoHistory();
    numInstancesParameter = 1;
    parameterSlotNumber = 16;
    selectedPreset = "---INIT---";

    notesMidiSeq.clear();
    ccMidiSeq.clear();

    ccBase = 102;
    psModSource = USE_NONE;
    midiChannel = 0;

    properlyPrepared = false;
    isPlayingNote = false;
    panicNoteOff = false;
    isWithIn = -1;
    wasStopped = true;
    playingNotes.clear();

    isHostPlaying = false;
    transportTimeElasped = 0;
    transportState = PLUGIN_STOPPED;

    nextStartTime = -1.0;
    rangeStartTime = FLT_MAX;
    rangeEndTime = -FLT_MAX;
    sampleLength = -1;
    bufferLength = -1;
    updateMidSeqSus = false;
    updateModSrcSus = false;
    pluginLoadSus = false;
}

void MicroChromoAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    auto validDesc = [](PluginDescription desc) {
        return desc.name.isNotEmpty() && desc.uid != 0;
    };

    //auto state = parameters.copyState();

    std::unique_ptr<XmlElement> xml = std::make_unique<XmlElement>("root");
    xml->setAttribute("lastDocumentFilename", lastDocumentFilename);
    xml->setAttribute("numInstances", numInstancesParameter);
    xml->setAttribute("midiChannel", midiChannel);
    xml->setAttribute("ccBase", ccBase);
    xml->setAttribute("isModSrcKontakt", psModSource == USE_KONTAKT ? 1 : 0);
    xml->setAttribute("selectedPreset", selectedPreset);

    //xml->addChildElement(new XmlElement(*state.createXml()));
    xml->addChildElement(new XmlElement(*synthBundle->createXml("synthBundle")));
    xml->addChildElement(new XmlElement(*psBundle->createXml("pitchshiftBundle")));

    auto* projectNode = new XmlElement(*project.serialize().createXml());
    xml->addChildElement(projectNode);

    copyXmlToBinary(*xml, destData);
    xml->writeTo(File("J:\\TEMPS\\13-SP20\\MUS 499\\Test.xml"));
}

void MicroChromoAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto validDesc = [](PluginDescription desc) {
        return desc.name.isNotEmpty() && desc.uid != 0;
    };

    std::unique_ptr<XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml.get() != nullptr)
    {
        if (xml->hasTagName("root"))
        {
            lastDocumentFilename = xml->getStringAttribute("lastDocumentFilename", "");
            synthBundle->closeAllWindows();
            psBundle->closeAllWindows();
            numInstancesParameter = xml->getIntAttribute("numInstances", 1);
            updateMidiChannel(xml->getIntAttribute("midiChannel", 1));
            updateCcMidiSequenceWithNewBase(xml->getIntAttribute("ccBase", 102));
            selectedPreset = xml->getStringAttribute("selectedPreset", "---INIT---");

            auto isModSrcKontakt = xml->getIntAttribute("isModSrcKontakt", 0);
            if (auto* child = xml->getChildByName("synthBundle"))
            {
                PluginDescription desc;
                desc.loadFromXml(*(child->getChildByName("PLUGIN")));
                if (validDesc(desc))
                {
                    this->addPlugin(desc, true, [copyChild = *child, isModSrcKontakt, this](PluginBundle& bundle) {
                        bundle.loadFromXml(&copyChild, "synthBundle");
                        if (isModSrcKontakt)
                            this->toggleUseKontakt(true);
                        });
                }
            }

            if (auto* child = xml->getChildByName("pitchshiftBundle"))
            {
                PluginDescription desc;
                desc.loadFromXml(*(child->getChildByName("PLUGIN")));
                if (validDesc(desc))
                {
                    this->addPlugin(desc, false, [copyChild = *child](PluginBundle& bundle) {
                        bundle.loadFromXml(&copyChild, "pitchshiftBundle");
                        });
                }
            }

            if (auto* child = xml->getChildByName("project"))
            {
                project.deserialize(ValueTree::fromXml(*child));
            }
        }
    }
}

void MicroChromoAudioProcessor::changeListenerCallback(ChangeBroadcaster* changed)
{
    if (changed == &knownPluginList)
    {
        synthKnownPluginList.clear();
        psKnownPluginList.clear();
        for (auto& type : knownPluginList.getTypes())
        {
            if (type.isInstrument)
                synthKnownPluginList.addType(type);
            else
                psKnownPluginList.addType(type);
        }
    }
}

void MicroChromoAudioProcessor::addPlugin(const PluginDescription& desc, bool isSynth, std::function<void(PluginBundle&)> callback)
{
    if (isSynth)
    {
        synthBundle->loadPlugin(desc, numInstancesParameter, [callback, this](PluginBundle& bundle)
            {
                if (callback)
                    callback(bundle);
            });
    }
    else
    {
        psBundle->loadPlugin(desc, numInstancesParameter, [callback, this](PluginBundle& bundle)
            {
                if (callback)
                    callback(bundle);
            });
    }
}

void MicroChromoAudioProcessor::startLoadingPlugin()
{
    pluginLoadSus = true;
    suspendProcessing(pluginLoadSus || updateModSrcSus || updateMidSeqSus || loadingDocument);
}

void MicroChromoAudioProcessor::finishLoadingPlugin()
{
    if (synthBundle->finishedLoading() && psBundle->finishedLoading())
    {
        if (properlyPrepared.load())
        {
            prepareToPlay(getSampleRate(), getBlockSize());
            updateHostDisplay();
        }

        updatePitchShiftModulationSource();

        pluginLoadSus = false;
        suspendProcessing(pluginLoadSus || updateModSrcSus || updateMidSeqSus || loadingDocument);
    }
}

void MicroChromoAudioProcessor::togglePlayback()
{
    ScopedLock lock(transportLock);
    if (transportState == PLUGIN_PAUSED || transportState == PLUGIN_STOPPED)
        transportState = PLUGIN_QUERY_PLAY;
    else if (transportState == PLUGIN_PLAYING)
        transportState = PLUGIN_QUERY_PAUSE;
}

void MicroChromoAudioProcessor::stopPlayback()
{
    ScopedLock lock(transportLock);
    if (transportState == PLUGIN_PLAYING || transportState == PLUGIN_PAUSED)
        transportState = PLUGIN_QUERY_STOP;
}

void MicroChromoAudioProcessor::setTimeForPlayback(double time)
{
    ScopedLock lock(transportLock);
    transportTimeElasped = time;
}

int MicroChromoAudioProcessor::getTransportState()
{
    return transportState;
}

double MicroChromoAudioProcessor::getTimeElapsed()
{
    return transportTimeElasped;
}

void MicroChromoAudioProcessor::updatePitchShiftModulationSource(int useKontakt)
{
    updateModSrcSus = true;
    suspendProcessing(pluginLoadSus || updateModSrcSus || updateMidSeqSus || loadingDocument);

    auto& synthCcLearnModule = synthBundle->getCcLearnModule();
    auto& psCcLearnModule = psBundle->getCcLearnModule();

    if (useKontakt == 1 && synthBundle->isKontakt())
    {
        psModSource = USE_KONTAKT;
        if (synthCcLearnModule.hasLearned())
            synthCcLearnModule.resetCcLearn();
        if (psCcLearnModule.hasLearned())
            psCcLearnModule.resetCcLearn();
        if (!isCurrentModSrcKontakt)
            updateMidiSequence(jlimit(0, 116, ccBase.load()));
        DBG("Mod src now is: Kontakt");
    }
    else
    {
        if (useKontakt == 1 || (useKontakt == 0 && psModSource == USE_KONTAKT && !synthBundle->isKontakt()))
            psModSource = USE_NONE;

        if (useKontakt == -1 || psModSource != USE_KONTAKT)
        {
            if (synthCcLearnModule.hasLearned())
            {
                if (psCcLearnModule.hasLearned())
                    psCcLearnModule.resetCcLearn();

                psModSource = USE_SYNTH;
                if (isCurrentModSrcKontakt)
                    updateMidiSequence(synthCcLearnModule.getCcSource());
                else
                    updateCcMidiSequenceWithNewBase(synthCcLearnModule.getCcSource());
                DBG("Mod src now is: Synth " << ccBase);
            }
            else if (psCcLearnModule.hasLearned())
            {
                psModSource = USE_PS;
                if (isCurrentModSrcKontakt)
                    updateMidiSequence(psCcLearnModule.getCcSource());
                else
                    updateCcMidiSequenceWithNewBase(psCcLearnModule.getCcSource());
                DBG("Mod src now is: Ps " << ccBase);
            }
            else
            {
                psModSource = USE_NONE;
                DBG("Mod src now is: None");
            }
        }
    }

    updateModSrcSus = false;
    suspendProcessing(pluginLoadSus || updateModSrcSus || updateMidSeqSus || loadingDocument);
}

bool MicroChromoAudioProcessor::canLearnCc(const PluginBundle* bundle)
{
    if (psModSource == USE_KONTAKT)
        return false;
    if (synthBundle.get() == bundle && psBundle->getCcLearnModule().hasLearned())
        return false;
    if (psBundle.get() == bundle && synthBundle->getCcLearnModule().hasLearned())
        return false;
    return true;
}

bool MicroChromoAudioProcessor::canChooseKontakt()
{
    if (!synthBundle->getCcLearnModule().hasLearned() && !psBundle->getCcLearnModule().hasLearned())
        return true;
    return false;
}

void MicroChromoAudioProcessor::toggleUseKontakt(bool isOn)
{
    if (!synthBundle->isKontakt())
        return;
    if (isOn)
        updatePitchShiftModulationSource(1);
    else
        updatePitchShiftModulationSource(-1);
}

int MicroChromoAudioProcessor::getPitchShiftModulationSource()
{
    return psModSource;
}

void MicroChromoAudioProcessor::updateMidiChannel(int newMidiChannel)
{
    if (midiChannel == newMidiChannel)
        return;

    midiChannel = newMidiChannel;
}

void MicroChromoAudioProcessor::updateKontaktCcBase(int newCcBase)
{
    if (psModSource != USE_KONTAKT)
    {
        AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, "Error", "Not using Kontakt but trying to specifying CC Base");
        return;
    }
    updateCcMidiSequenceWithNewBase(newCcBase);
}

//===------------------------------------------------------------------===//
// FileBasedDocument
//===------------------------------------------------------------------===//
String MicroChromoAudioProcessor::getDocumentTitle()
{
    return project.getTitle();
}

Result MicroChromoAudioProcessor::loadDocument(const File& file)
{
    FileInputStream ss(file);
    if (ss.openedOk())
    {
        MemoryBlock block;
        ss.readIntoMemoryBlock(block);

        loadingDocument = true;
        suspendProcessing(pluginLoadSus || updateModSrcSus || updateMidSeqSus || loadingDocument);

        stopTimer();
        reset();
        setStateInformation(block.getData(), block.getSize());
        project.broadcastReloadProjectContent();

        prepareToPlay(getSampleRate(), getBlockSize());

        startTimer(10);
        loadingDocument = false;
        suspendProcessing(pluginLoadSus || updateModSrcSus || updateMidSeqSus || loadingDocument);


        return Result::ok();
    }
    else
        return Result::fail("Cannot open file");
}

Result MicroChromoAudioProcessor::saveDocument(const File& file)
{
    MemoryBlock block;
    getStateInformation(block);
    FileOutputStream ss(file);
    if (ss.openedOk())
    {
        ss.setPosition(0);
        ss.truncate();
        if (ss.write(block.getData(), block.getSize()))
            return Result::ok();
        else
            return Result::fail("Write failure");
    }
    else
        return Result::fail("Open failure");
}

File MicroChromoAudioProcessor::getLastDocumentOpened()
{
    if (lastDocumentFilename.isEmpty())
        return File::getSpecialLocation(File::userDocumentsDirectory);
    else
        return File(lastDocumentFilename);
}

void MicroChromoAudioProcessor::setLastDocumentOpened(const File& file)
{
    lastDocumentFilename = file.getFullPathName();
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MicroChromoAudioProcessor();
}

