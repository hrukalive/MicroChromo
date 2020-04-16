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
       parameters(*this, nullptr)
#endif
{
    PropertiesFile::Options options;
    options.folderName = "MicroChromo";
    options.applicationName = "MicroChromo";
    options.filenameSuffix = "settings";
    options.osxLibrarySubFolder = "Preferences";
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

    synthBundle->loadPluginSync(internalTypes[1], numInstancesParameter);
    psBundle->loadPluginSync(internalTypes[2], numInstancesParameter);

    controllerStateMessage.ensureStorageAllocated(128);

    project.reset(new Project(*this, "Untitled"));

    startTimer(10);

    noteColorMap.set("0", ColorPitchBendRecord("0", 0, Colours::grey));

    notes.add(Note(nullptr, 60, 2, 2, 0.8f, "0"));
    notes.add(Note(nullptr, 63, 2, 2, 0.8f, "0"));
    notes.add(Note(nullptr, 67, 2, 2, 0.8f, "0"));
    notes.add(Note(nullptr, 60 + 12, 2, 2, 0.8f, "0"));
    notes.add(Note(nullptr, 63 + 12, 2, 2, 0.8f, "0"));
    notes.add(Note(nullptr, 67 + 12, 2, 2, 0.8f, "0"));

    notes.add(Note(nullptr, 60, 5, 2, 0.8f, "0"));
    notes.add(Note(nullptr, 63, 5, 2, 0.8f, "0"));
    notes.add(Note(nullptr, 67, 5, 2, 0.8f, "0"));

    notes.add(Note(nullptr, 60, 8, 3, 0.8f, "0"));
    notes.add(Note(nullptr, 64, 8, 3, 0.8f, "0"));
    notes.add(Note(nullptr, 67, 8, 3, 0.8f, "0"));
    //notes.add(Note(60, 2, 1, 1));
    //notes.add(Note(61, 4, 1, 1));
    //notes.add(Note(62, 6, 2, 1));
    //notes.add(Note(63, 7, 2, 1));
    //notes.add(Note(64, 8, 2, 1));
    //notes.add(Note(65, 9, 2, 1));
    //notes.add(Note(66, 10, 2, 1));
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
            midiBuffer->addEvent(MidiMessage::allNotesOff(j), 0);
            midiBuffer->addEvent(MidiMessage::allSoundOff(j), 0);
            midiBuffer->addEvent(MidiMessage::allControllersOff(j), 0);
            for (auto n : playingNotes)
                midiBuffer->addEvent(MidiMessage::noteOff(j, n), 0);
            playingNotes.clear();
        }
    }
    for (auto* midiBuffer : midiBufferArrayB)
    {
        midiBuffer->clear();
        for (auto j = 1; j <= 16; j++)
        {
            midiBuffer->addEvent(MidiMessage::allNotesOff(j), 0);
            midiBuffer->addEvent(MidiMessage::allSoundOff(j), 0);
            midiBuffer->addEvent(MidiMessage::allControllersOff(j), 0);
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
            midiBuffer->addEvent(MidiMessage::allNotesOff(j), 0);
            midiBuffer->addEvent(MidiMessage::allSoundOff(j), 0);
            midiBuffer->addEvent(MidiMessage::allControllersOff(j), 0);
            for (int k = 0; k < 128; k++)
                midiBuffer->addEvent(MidiMessage::noteOff(j, k), 1);
            playingNotes.clear();
        }
    }
    for (auto* midiBuffer : midiBufferArrayB)
    {
        midiBuffer->clear();
        for (auto j = 1; j <= 16; j++)
        {
            midiBuffer->addEvent(MidiMessage::allNotesOff(j), 0);
            midiBuffer->addEvent(MidiMessage::allSoundOff(j), 0);
            midiBuffer->addEvent(MidiMessage::allControllersOff(j), 0);
            for (int k = 0; k < 128; k++)
                midiBuffer->addEvent(MidiMessage::noteOff(j, k), 1);
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
            transportState = PLUGIN_PAUSED;
    }
    else
        transportState = HOST_PLAYING;
}

void MicroChromoAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
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
            controllerStateMessage.clear();
            ccMidiSeq[i]->createControllerUpdatesForTime(midiChannel, startTime, controllerStateMessage);
            for (auto& msg : controllerStateMessage)
            {
                ccMidiBufferArray[i]->addEvent(msg, 8);
                DBG("Recovering CC: " << msg.getControllerNumber() << ": " << msg.getControllerValue());
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
            DBG("Playing just now, and now stopped");
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
        if (nextStartTime > 0.0 && std::abs(startTime - nextStartTime) > 2 * bufferLength)
        {
            if (!noteOffSent)
            {
                sendAllNotesOff();
                ensureCcValue();
                noteOffSent = true;
            }
            DBG("Playhead moved by cursor or looping");
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
            DBG("Playing range entered or exited.");
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
            DBG("Stopped before, now playing.");
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
                                midiBufferArrayA[i]->addEvent(evt->message, 16);
                                if (evt->message.isNoteOn())
                                    playingNotes.insert(evt->message.getNoteNumber());
                                else if (evt->message.isNoteOff())
                                    playingNotes.erase(evt->message.getNoteNumber());
                                DBG("Retrigger note " << evt->message.getNoteNumber());
                                isPlayingNote = true;
                            }
                        }
                    }
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
                                playingNotes.insert(evt->message.getNoteNumber());
                            else if (evt->message.isNoteOff())
                                playingNotes.erase(evt->message.getNoteNumber());
                            isPlayingNote = true;
                        }
                        if (evt->message.getTimeStamp() >= endTime)
                            break;
                    }
                    for (int j = ccMidiSeq[i]->getNextIndexAtTime(startTime); j < ccMidiSeq[i]->getNumEvents(); j++)
                    {
                        MidiMessageSequence::MidiEventHolder* evt = ccMidiSeq[i]->getEventPointer(j);

                        if (evt->message.getTimeStamp() >= startTime && evt->message.getTimeStamp() < endTime)
                        {
                            ccMidiBufferArray[i]->addEvent(evt->message, roundDoubleToInt((evt->message.getTimeStamp() - startTime) * sampleLength));
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
    suspendProcessing(pluginLoadSus || updateModSrcSus || updateMidSeqSus);

    if (newBase != -1 && ccBase != newBase)
        ccBase = newBase;

    sortNotes();

    notesMidiSeq.clear();
    ccMidiSeq.clear();
    for (int i = 0; i < numInstancesParameter; i++)
    {
        notesMidiSeq.add(new MidiMessageSequence());
        ccMidiSeq.add(new MidiMessageSequence());
        for (int j = 0; j < (psModSource == USE_KONTAKT ? 12 : 1); j++)
            ccMidiSeq[i]->addEvent(MidiMessage::controllerEvent(midiChannel, ccBase + j, 50));
    }

    Array<SimpleMidiMessage> sequence;
    for (auto& note : notes)
    {
        int pitchbend = 0;
        if (noteColorMap.contains(note.getPitchColor()))
            pitchbend = noteColorMap[note.getPitchColor()].value;
        auto key = note.getKey();
        while (pitchbend > 50)
        {
            key++;
            pitchbend -= 100;
        }
        while (pitchbend < -50)
        {
            key--;
            pitchbend += 100;
        }
        key = jmax(0, key);
        if (psModSource == USE_KONTAKT)
            sequence.add(SimpleMidiMessage(midiChannel, key, note.getBeat(), note.getVelocity(), pitchbend, ccBase + key % 12, true, 2 * sampleLength));
        else
            sequence.add(SimpleMidiMessage(midiChannel, key, note.getBeat(), note.getVelocity(), pitchbend, ccBase, true, 2 * sampleLength));
        sequence.add(SimpleMidiMessage(midiChannel, key, note.getBeat() + note.getLength(), note.getVelocity(), 0, -1, false, 0));
    }
    sequence.sort(SimpleMidiMessageComparator(), true);

    if (psModSource == USE_KONTAKT)
        updateMidiSequenceKontakt(sequence);
    else
        updateMidiSequenceGeneral(sequence);

    for (int i = 0; i < notesMidiSeq.size(); i++)
        for (int j = 0; j < (psModSource == USE_KONTAKT ? 12 : 1); j++)
            ccMidiSeq[i]->addEvent(MidiMessage::controllerEvent(midiChannel, ccBase + j, 50).withTimeStamp(notesMidiSeq[i]->getEndTime() + 5));

    rangeStartTime = FP_INFINITE;
    rangeEndTime = -FP_INFINITE;
    for (auto& seq : notesMidiSeq)
    {
        if (rangeStartTime > seq->getStartTime())
            rangeStartTime = seq->getStartTime();
        if (rangeEndTime < seq->getEndTime())
            rangeEndTime = seq->getEndTime();
    }
    rangeEndTime += 4;

    updateMidSeqSus = false;
    suspendProcessing(pluginLoadSus || updateModSrcSus || updateMidSeqSus);
}

void MicroChromoAudioProcessor::updateMidiSequenceGeneral(Array<SimpleMidiMessage>& sequence)
{
    std::unordered_map<int, std::unordered_set<int>> channelToNote;
    std::unordered_map<int, int> channelToCcValue;
    std::unordered_map<int, int> noteToChannel;

    int channelUseCounter = 0;
    std::map<int, int> channelUseCount;
    for (int i = 0; i < numInstancesParameter; i++)
    {
        channelToNote[i] = std::unordered_set<int>();
        channelToCcValue[i] = -2;
        channelUseCount[i] = 0;
    }

    for (auto& note : sequence)
    {
        bool needFindEmpty = true;
        bool needOverride = true;
        if (note.noteMsg.isNoteOn())
        {
            for (int i = 0; i < numInstancesParameter; i++)
            {
                if (note.ccMsg.getControllerValue() == channelToCcValue[i])
                {
                    notesMidiSeq[i]->addEvent(note.noteMsg);

                    channelToNote[i].insert(note.noteMsg.getNoteNumber());
                    noteToChannel[note.noteMsg.getNoteNumber()] = i;

                    channelUseCount[i] = ++channelUseCounter;

                    needFindEmpty = false;
                    needOverride = false;

                    DBG("Sameaddv " << i << " " << note.toString());
                    break;
                }
            }
            if (needFindEmpty)
            {
                int minIdx = -1, minVal = INT_MAX;
                for (auto& v : channelUseCount)
                {
                    if (v.second < minVal && channelToNote[v.first].empty())
                    {
                        minIdx = v.first;
                        minVal = v.second;
                    }
                }
                if (minIdx != -1)
                {
                    notesMidiSeq[minIdx]->addEvent(note.noteMsg);
                    ccMidiSeq[minIdx]->addEvent(note.ccMsg);

                    channelToNote[minIdx].insert(note.noteMsg.getNoteNumber());
                    channelToCcValue[minIdx] = note.ccMsg.getControllerValue();
                    noteToChannel[note.noteMsg.getNoteNumber()] = minIdx;

                    channelUseCount[minIdx] = ++channelUseCounter;

                    needOverride = false;

                    DBG("Addfreev " << minIdx << " " << note.toString());
                    DBG("Addccccc " << minIdx << " " << note.toString());
                }
            }
            if (needOverride)
            {
                int minIdx = -1, minVal = INT_MAX;
                for (auto& v : channelUseCount)
                {
                    if (v.second < minVal)
                    {
                        minIdx = v.first;
                        minVal = v.second;
                    }
                }

                auto& allNotesNow = channelToNote[minIdx];
                for (auto noteNum : allNotesNow)
                {
                    MidiMessage msg = MidiMessage::noteOff(midiChannel, noteNum).withTimeStamp(note.noteMsg.getTimeStamp());
                    notesMidiSeq[minIdx]->addEvent(msg);
                    noteToChannel.erase(noteNum);
                }
                channelToNote[minIdx].clear();

                notesMidiSeq[minIdx]->addEvent(note.noteMsg);
                ccMidiSeq[minIdx]->addEvent(note.ccMsg);

                channelToNote[minIdx].insert(note.noteMsg.getNoteNumber());
                channelToCcValue[minIdx] = note.ccMsg.getControllerValue();
                noteToChannel[note.noteMsg.getNoteNumber()] = minIdx;

                channelUseCount[minIdx] = ++channelUseCounter;

                DBG("Override " << minIdx << " " << note.toString());
            }
        }
        else
        {
            if (noteToChannel.find(note.noteMsg.getNoteNumber()) != noteToChannel.end())
            {
                auto channel = noteToChannel[note.noteMsg.getNoteNumber()];
                notesMidiSeq[channel]->addEvent(note.noteMsg);
                noteToChannel.erase(note.noteMsg.getNoteNumber());
                channelToNote[channel].erase(note.noteMsg.getNoteNumber());

                DBG("Noteoffe " << channel << " " << note.toString());
            }
        }
    }

    for (auto& seq : notesMidiSeq)
    {
        seq->sort();
        seq->updateMatchedPairs();
    }
    for (auto& seq : ccMidiSeq)
    {
        seq->sort();
        seq->updateMatchedPairs();
    }

    isCurrentModSrcKontakt = false;
}

void MicroChromoAudioProcessor::updateMidiSequenceKontakt(Array<SimpleMidiMessage>& sequence)
{
    std::unordered_map<int, std::unordered_set<int>> channelToNote;
    std::unordered_map<int, int> channelToCcValue;
    std::unordered_map<int, int> noteToChannel;

    int channelUseCounter = 0;
    std::map<int, int> channelUseCount;
    for (int i = 0; i < numInstancesParameter * 12; i++)
    {
        channelToNote[i] = std::unordered_set<int>();
        channelToCcValue[i] = -2;
        channelUseCount[i] = 0;
    }

    for (auto& note : sequence)
    {
        bool needFindEmpty = true;
        bool needOverride = true;
        if (note.noteMsg.isNoteOn())
        {
            for (int i = 0; i < numInstancesParameter; i++)
            {
                auto realIdx = i * 12 + note.noteMsg.getNoteNumber() % 12;
                if (note.ccMsg.getControllerValue() == channelToCcValue[realIdx])
                {
                    notesMidiSeq[i]->addEvent(note.noteMsg);

                    channelToNote[realIdx].insert(note.noteMsg.getNoteNumber());
                    noteToChannel[note.noteMsg.getNoteNumber()] = realIdx;

                    channelUseCount[realIdx] = ++channelUseCounter;

                    needFindEmpty = false;
                    needOverride = false;

                    DBG("Sameaddv " << i << " " << realIdx % 12 << note.toString());
                    break;
                }
            }
            if (needFindEmpty)
            {
                int minIdx = -1, minVal = INT_MAX;
                for (int i = note.noteMsg.getNoteNumber() % 12; i < numInstancesParameter * 12; i += 12)
                {
                    auto v = channelUseCount[i];
                    if (v < minVal && channelToNote[i].empty())
                    {
                        minIdx = i;
                        minVal = v;
                    }
                }
                int seqIndex = minIdx / 12;

                if (minIdx != -1)
                {
                    notesMidiSeq[seqIndex]->addEvent(note.noteMsg);
                    ccMidiSeq[seqIndex]->addEvent(note.ccMsg);

                    channelToNote[minIdx].insert(note.noteMsg.getNoteNumber());
                    channelToCcValue[minIdx] = note.ccMsg.getControllerValue();
                    noteToChannel[note.noteMsg.getNoteNumber()] = minIdx;

                    channelUseCount[minIdx] = ++channelUseCounter;

                    needOverride = false;

                    DBG("Addfreev " << seqIndex << " " << minIdx % 12 << note.toString());
                    DBG("Addccccc " << seqIndex << " " << minIdx % 12 << note.toString());
                }
            }
            if (needOverride)
            {
                int minIdx = -1, minVal = INT_MAX;
                for (int i = note.noteMsg.getNoteNumber() % 12; i < numInstancesParameter * 12; i += 12)
                {
                    auto v = channelUseCount[i];
                    if (v < minVal)
                    {
                        minIdx = i;
                        minVal = v;
                    }
                }
                int seqIndex = minIdx / 12;

                auto& allNotesNow = channelToNote[minIdx];
                for (auto noteNum : allNotesNow)
                {
                    MidiMessage msg = MidiMessage::noteOff(midiChannel, noteNum).withTimeStamp(note.noteMsg.getTimeStamp());
                    notesMidiSeq[seqIndex]->addEvent(msg);
                    noteToChannel.erase(noteNum);
                }
                channelToNote[minIdx].clear();

                notesMidiSeq[seqIndex]->addEvent(note.noteMsg);
                ccMidiSeq[seqIndex]->addEvent(note.ccMsg);

                channelToNote[minIdx].insert(note.noteMsg.getNoteNumber());
                channelToCcValue[minIdx] = note.ccMsg.getControllerValue();
                noteToChannel[note.noteMsg.getNoteNumber()] = minIdx;

                channelUseCount[minIdx] = ++channelUseCounter;

                DBG("Override " << seqIndex << " " << minIdx % 12 << note.toString());
            }
        }
        else
        {
            if (noteToChannel.find(note.noteMsg.getNoteNumber()) != noteToChannel.end())
            {
                auto channel = noteToChannel[note.noteMsg.getNoteNumber()];
                notesMidiSeq[channel / 12]->addEvent(note.noteMsg);
                noteToChannel.erase(note.noteMsg.getNoteNumber());
                channelToNote[channel].erase(note.noteMsg.getNoteNumber());

                DBG("Noteoffe " << channel / 12 << " " << channel % 12 << note.toString());
            }
        }
    }

    for (auto& seq : notesMidiSeq)
    {
        seq->sort();
        seq->updateMatchedPairs();
    }
    for (auto& seq : ccMidiSeq)
    {
        seq->sort();
        seq->updateMatchedPairs();
    }

    isCurrentModSrcKontakt = true;
}

void MicroChromoAudioProcessor::updateNoteColorMap(Array<ColorPitchBendRecord>& colors)
{
    noteColorMap.clear();
    for (auto c : colors)
        noteColorMap.set(c.name, c);
    if (!noteColorMap.contains("0"))
        noteColorMap.set("0", ColorPitchBendRecord("0", 0, Colours::grey));
    filterNotesWithColorMap();
}

void MicroChromoAudioProcessor::renameNoteColorMap(String oldName, String newName)
{
    if (noteColorMap.contains(oldName))
    {
        auto c = noteColorMap[oldName];
        c.name = newName;
        noteColorMap.remove(oldName);
        noteColorMap.set(newName, c);

        for (auto& note : notes)
        {
            if (note.getPitchColor() == oldName)
                note.setPitchColor(newName);
        }
    }
}

void MicroChromoAudioProcessor::clearNoteColorMap()
{
    updateNoteColorMap(Array<ColorPitchBendRecord>());
}

void MicroChromoAudioProcessor::filterNotesWithColorMap()
{
    for (auto& note : notes)
    {
        if (!noteColorMap.contains(note.getPitchColor()))
            note.setPitchColor("0");
    }
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
void MicroChromoAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    auto validDesc = [](PluginDescription desc) {
        return desc.name.isNotEmpty() && desc.uid != 0;
    };

    //auto state = parameters.copyState();
    //auto stateXml = new XmlElement(*state.createXml());

    std::unique_ptr<XmlElement> xml = std::make_unique<XmlElement>("root");
    xml->setAttribute("numInstances", numInstancesParameter);
    xml->setAttribute("midiChannel", midiChannel);
    xml->setAttribute("ccBase", ccBase);
    xml->setAttribute("isModSrcKontakt", psModSource == USE_KONTAKT ? 1 : 0);
    xml->setAttribute("selectedPreset", selectedPreset);

    //xml->addChildElement(stateXml);
    xml->addChildElement(new XmlElement(*synthBundle->createXml("synthBundle")));
    xml->addChildElement(new XmlElement(*psBundle->createXml("pitchshiftBundle")));

    Array<ColorPitchBendRecord> colorMaps;
    HashMap<String, ColorPitchBendRecord>::Iterator it(noteColorMap);
    while (it.next())
        colorMaps.add(it.getValue());
    colorMaps.sort(ColorPitchBendRecordComparator());
    xml->addChildElement(ColorPitchBendRecordCollection::getColorMapXml("", colorMaps));

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

            if (auto* child = xml->getChildByName("colorMap"))
            {
                Array<ColorPitchBendRecord> colors;
                forEachXmlChildElementWithTagName(*child, record, "record")
                {
                    colors.add(ColorPitchBendRecord(record->getStringAttribute("name", "Unknown"),
                        record->getDoubleAttribute("value", 0),
                        Colour::fromString(record->getStringAttribute("color", "ff000000"))));
                }
                colors.sort(ColorPitchBendRecordComparator());
                updateNoteColorMap(colors);
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
    suspendProcessing(pluginLoadSus || updateModSrcSus || updateMidSeqSus);
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
        suspendProcessing(pluginLoadSus || updateModSrcSus || updateMidSeqSus);
    }
}


void MicroChromoAudioProcessor::addNote(const Note& note)
{
    notes.add(note);
}
void MicroChromoAudioProcessor::clearNotes()
{
    notes.clear();
}
void MicroChromoAudioProcessor::insertNote(int index, const Note& note)
{
    notes.insert(index, note);
}
void MicroChromoAudioProcessor::sortNotes()
{
    notes.sort(NoteComparator(), true);
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
    suspendProcessing(pluginLoadSus || updateModSrcSus || updateMidSeqSus);

    auto& synthCcLearnModule = synthBundle->getCcLearnModule();
    auto& psCcLearnModule = psBundle->getCcLearnModule();

    if (useKontakt == 1)
    {
        if (synthBundle->isKontakt())
        {
            psModSource = USE_KONTAKT;
            if (synthCcLearnModule.hasLearned())
                synthCcLearnModule.reset();
            if (psCcLearnModule.hasLearned())
                psCcLearnModule.reset();
            if (!isCurrentModSrcKontakt)
                updateMidiSequence(jlimit(0, 116, ccBase.load()));
            DBG("Mod src now is: Kontakt");
            return;
        }
        psModSource = USE_NONE;
    }
    if (useKontakt == 0 && psModSource == USE_KONTAKT && !synthBundle->isKontakt())
        psModSource = USE_NONE;

    if (useKontakt == -1 || psModSource != USE_KONTAKT)
    {
        if (synthCcLearnModule.hasLearned())
        {
            if (psCcLearnModule.hasLearned())
                psCcLearnModule.reset();

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

    updateModSrcSus = false;
    suspendProcessing(pluginLoadSus || updateModSrcSus || updateMidSeqSus);
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

    for (int i = 0; i < numInstancesParameter; i++)
    {
        for (auto* note : *notesMidiSeq[i])
            note->message.setChannel(newMidiChannel);
        for (auto* cc : *ccMidiSeq[i])
            cc->message.setChannel(newMidiChannel);
    }
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

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MicroChromoAudioProcessor();
}

