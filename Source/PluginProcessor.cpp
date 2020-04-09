/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "InternalPlugins.h"

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

    noteColorMap.set("0", ColorPitchBendRecord("0", 0, Colours::grey));

    notes.add(Note(60, 2, 2, 0.8f, "0"));
    notes.add(Note(63, 2, 2, 0.8f, "0"));
    notes.add(Note(67, 2, 2, 0.8f, "0"));
    notes.add(Note(60 + 12, 2, 2, 0.8f, "0"));
    notes.add(Note(63 + 12, 2, 2, 0.8f, "0"));
    notes.add(Note(67 + 12, 2, 2, 0.8f, "0"));

    notes.add(Note(60, 5, 2, 0.8f, "0"));
    notes.add(Note(63, 5, 2, 0.8f, "0"));
    notes.add(Note(67, 5, 2, 0.8f, "0"));

    notes.add(Note(60, 8, 3, 0.8f, "0"));
    notes.add(Note(64, 8, 3, 0.8f, "0"));
    notes.add(Note(67, 8, 3, 0.8f, "0"));
}

MicroChromoAudioProcessor::~MicroChromoAudioProcessor()
{
    knownPluginList.removeChangeListener(this);
    synthParamPtr.clear();
    psParamPtr.clear();
    synthBundle = nullptr;
    psBundle = nullptr;
    bufferArrayA.clear();
    bufferArrayB.clear();
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

    bufferArrayA.clear();
    bufferArrayB.clear();
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
        bufferArrayA.add(new AudioBuffer<float>(aChannelNum, samplesPerBlock));
        bufferArrayB.add(new AudioBuffer<float>(bChannelNum, samplesPerBlock));
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
    bufferArrayA.clear();
    bufferArrayB.clear();
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

void MicroChromoAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;


    if (getPlayHead())
    {
        getPlayHead()->getCurrentPosition(posInfo);
        hasPlayheadInfo = true;
    }
    else
    {
        hasPlayheadInfo = false;
        posInfo.isPlaying = false;
    }

    auto startTime = posInfo.timeInSeconds;
    auto endTime = startTime + bufferLength;
    auto isWithInNow = (rangeStartTime < rangeEndTime && startTime >= rangeStartTime && startTime < rangeEndTime) ? 1 : 0;

    auto forwardMidiBuffer = [&]()
    {
        MidiMessage midiMessage;
        int sampleOffset;
        for (MidiBuffer::Iterator it(midiMessages); it.getNextEvent(midiMessage, sampleOffset);)
        {
            midiMessage.setTimeStamp(sampleOffset * sampleLength + sampleLength);
            synthBundle->getCollectorAt(0)->addMessageToQueue(midiMessage);
            psBundle->getCollectorAt(0)->addMessageToQueue(midiMessage);
        }
    };

    auto ensureCcValue = [&]()
    {
        if (ccBundle)
        {
            for (int i = 0; i < numInstancesParameter; i++)
            {
                controllerStateMessage.clear();
                ccMidiSeq[i]->createControllerUpdatesForTime(midiChannel, startTime, controllerStateMessage);
                for (auto& msg : controllerStateMessage)
                {
                    ccBundle->getCollectorAt(i)->addMessageToQueue(msg.withTimeStamp(sampleLength));
                    DBG("Recovering CC: " << msg.getControllerNumber() << ": " << msg.getControllerValue());
                }
            }
        }
    };

    // MIDI processing
    MidiBuffer midiBuffer;
    if (!posInfo.isPlaying)
    {
        if (isPlayingNote)
        {
            midiMessages.clear();
            DBG("Playing just now, and now stopped");
            sendAllNotesOff();
        }
        else
        {
            forwardMidiBuffer();
            midiMessages.clear();
        }
    }
    else
    {
        if (nextStartTime > 0.0 && std::abs(startTime - nextStartTime) > 2 * bufferLength)
        {
            sendAllNotesOff();
            ensureCcValue();
            DBG("Playhead moved by cursor or looping");
        }
        nextStartTime = endTime;

        if (isWithIn == -1 || isWithIn != isWithInNow)
        {
            sendAllNotesOff();
            isWithIn = isWithInNow;
            ensureCcValue();
            DBG("Playing range entered or exited.");
        }

        if (isWithInNow == 1)
        {
            MidiMessage midiMessage;
            int sampleOffset;
            for (MidiBuffer::Iterator it(midiMessages); it.getNextEvent(midiMessage, sampleOffset);)
            {
                if (!midiMessage.isNoteOnOrOff())
                {
                    midiMessage.setTimeStamp(sampleOffset * sampleLength + sampleLength);
                    if (psModSource == USE_PS)
                    {
                        synthBundle->addMessageToAllQueue(midiMessage);
                        if (!midiMessage.isControllerOfType(psBundle->getLearnedCcSource()))
                            psBundle->addMessageToAllQueue(midiMessage);
                    }
                    else if (psModSource == USE_SYNTH)
                    {
                        if (!midiMessage.isControllerOfType(synthBundle->getLearnedCcSource()))
                            synthBundle->addMessageToAllQueue(midiMessage);
                        psBundle->addMessageToAllQueue(midiMessage);
                    }
                    else if (psModSource == USE_KONTAKT)
                    {
                        if (!midiMessage.isController() || 
                            midiMessage.getControllerNumber() < ccBase ||
                            midiMessage.getControllerNumber() >= ccBase + 12)
                            synthBundle->addMessageToAllQueue(midiMessage);
                        psBundle->addMessageToAllQueue(midiMessage);
                    }
                }
            }

            if (!isPlayingNote)
            {
                for (int i = 0; i < numInstancesParameter; i++)
                {
                    const auto noteEvtIndex = notesMidiSeq[i]->getNextIndexAtTime(startTime);
                    for (int j = 0; j < noteEvtIndex; j++)
                    {
                        MidiMessageSequence::MidiEventHolder* event = notesMidiSeq[i]->getEventPointer(j);
                        if (event->message.isNoteOn() && event->noteOffObject != nullptr && event->noteOffObject->message.getTimeStamp() > startTime)
                        {
                            synthBundle->getCollectorAt(i)->addMessageToQueue(event->message.withTimeStamp(event->message.getTimeStamp() - startTime + sampleLength));
                            isPlayingNote = true;
                        }
                    }
                }
            }

            for (int i = 0; i < numInstancesParameter; i++)
            {
                for (int j = notesMidiSeq[i]->getNextIndexAtTime(startTime); j < notesMidiSeq[i]->getNumEvents(); j++)
                {
                    MidiMessageSequence::MidiEventHolder* event = notesMidiSeq[i]->getEventPointer(j);

                    if (event->message.getTimeStamp() >= startTime && event->message.getTimeStamp() < endTime)
                    {
                        synthBundle->getCollectorAt(i)->addMessageToQueue(event->message.withTimeStamp(event->message.getTimeStamp() - startTime + sampleLength));
                        isPlayingNote = true;
                    }
                    if (event->message.getTimeStamp() >= endTime)
                        break;
                }
                if (ccBundle)
                {
                    const auto ccEvtIndex = ccMidiSeq[i]->getNextIndexAtTime(startTime);
                    if (ccEvtIndex - 1 > 0)
                    {
                        MidiMessageSequence::MidiEventHolder* event = ccMidiSeq[i]->getEventPointer(ccEvtIndex - 1);
                        ccBundle->getCollectorAt(i)->addMessageToQueue(event->message.withTimeStamp(sampleLength));
                    }
                    for (int j = ccEvtIndex; j < ccMidiSeq[i]->getNumEvents(); j++)
                    {
                        MidiMessageSequence::MidiEventHolder* event = ccMidiSeq[i]->getEventPointer(j);

                        if (event->message.getTimeStamp() >= startTime && event->message.getTimeStamp() < endTime)
                        {
                            ccBundle->getCollectorAt(i)->addMessageToQueue(event->message.withTimeStamp(event->message.getTimeStamp() - startTime + sampleLength));
                        }
                        if (event->message.getTimeStamp() >= endTime)
                            break;
                    }
                }
            }
            
        }
        else
        {
            forwardMidiBuffer();
            midiMessages.clear();
        }
    }

    // Synthesis
    for (auto i = 0; i < numInstancesParameter; i++)
    {
        int splitPoint = jmin(synthBundleMainBusNumInputChannels, 2, buffer.getNumChannels());
        for (int j = 0; j < splitPoint; j++)
            bufferArrayA[i]->copyFrom(j, 0, buffer, j, 0, buffer.getNumSamples());
        for (int j = splitPoint; j < synthBundleTotalNumInputChannels; j++)
            bufferArrayA[i]->clear(j, 0, bufferArrayA[i]->getNumSamples());
    }
    synthBundle->processBlock(bufferArrayA, midiBuffer);

    for (auto i = 0; i < numInstancesParameter; i++)
    {
        int splitPoint = jmin(2 + psBundleMainBusNumInputChannels, psBundleTotalNumInputChannels, buffer.getNumChannels());
        for (int j = 0; j < jmin(synthBundleMainBusNumOutputChannels, psBundleMainBusNumInputChannels); j++)
            bufferArrayB[i]->copyFrom(j, 0, *bufferArrayA[i], j, 0, bufferArrayA[i]->getNumSamples());
        for (int j = psBundleMainBusNumInputChannels; j < splitPoint; j++)
            bufferArrayB[i]->copyFrom(j, 0, buffer, j - psBundleMainBusNumInputChannels + 2, 0, buffer.getNumSamples());
        for (int j = splitPoint; j < psBundleTotalNumInputChannels; j++)
            bufferArrayB[i]->clear(j, 0, bufferArrayB[i]->getNumSamples());
    }
    psBundle->processBlock(bufferArrayB, midiBuffer);

    for (auto i = 0; i < buffer.getNumChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
    for (auto i = 0; i < numInstancesParameter; i++)
        for (auto j = 0; j < jmin(getMainBusNumOutputChannels(), psBundleMainBusNumOutputChannels); j++)
            buffer.addFrom(j, 0, *bufferArrayB[i], j, 0, buffer.getNumSamples());

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
        sequence.add(SimpleMidiMessage(midiChannel, key, note.getBeat(), note.getVelocity(), pitchbend, ccBase, true, 2 * sampleLength));
        sequence.add(SimpleMidiMessage(midiChannel, key, note.getBeat() + note.getLength(), note.getVelocity(), 0, -1, false, 0));
    }
    sequence.sort(SimpleMidiMessageComparator(), true);

    if (psModSource == USE_KONTAKT)
        updateMidiSequenceKontakt(sequence);
    else
        updateMidiSequenceGeneral(sequence);

    rangeStartTime = FP_INFINITE;
    rangeEndTime = -FP_INFINITE;
    for (auto& seq : notesMidiSeq)
    {
        if (rangeStartTime > seq->getStartTime())
            rangeStartTime = seq->getStartTime();
        if (rangeEndTime < seq->getEndTime())
            rangeEndTime = seq->getEndTime();
    }

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
        bool needOverride = true;
        if (note.noteMsg.isNoteOn())
        {
            for (int i = 0; i < numInstancesParameter; i++)
            {
                if (channelToNote[i].empty())
                {
                    notesMidiSeq[i]->addEvent(note.noteMsg);
                    ccMidiSeq[i]->addEvent(note.ccMsg);

                    channelToNote[i].insert(note.noteMsg.getNoteNumber());
                    channelToCcValue[i] = note.ccMsg.getControllerValue();
                    noteToChannel[note.noteMsg.getNoteNumber()] = i;

                    channelUseCount[i] = channelUseCounter++;

                    needOverride = false;

                    DBG("Addfreev " << i << " " << note.toString());
                    DBG("Addccccc " << i << " " << note.toString());
                    break;
                }
                else if (note.ccMsg.getControllerValue() == channelToCcValue[i])
                {
                    notesMidiSeq[i]->addEvent(note.noteMsg);

                    channelToNote[i].insert(note.noteMsg.getNoteNumber());
                    noteToChannel[note.noteMsg.getNoteNumber()] = i;

                    channelUseCount[i] = channelUseCounter++;

                    needOverride = false;
                     
                    DBG("Sameaddv " << i << " " << note.toString());
                    break;
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

                channelUseCount[minIdx] = channelUseCounter++;

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
        bool needOverride = true;
        if (note.noteMsg.isNoteOn())
        {
            for (int i = 0; i < numInstancesParameter; i++)
            {
                auto realIdx = i * 12 + note.noteMsg.getNoteNumber() % 12;
                if (channelToNote[realIdx].empty())
                {
                    notesMidiSeq[i]->addEvent(note.noteMsg);
                    ccMidiSeq[i]->addEvent(note.ccMsg);

                    channelToNote[realIdx].insert(note.noteMsg.getNoteNumber());
                    channelToCcValue[realIdx] = note.ccMsg.getControllerValue();
                    noteToChannel[note.noteMsg.getNoteNumber()] = realIdx;

                    channelUseCount[realIdx] = channelUseCounter++;

                    needOverride = false;

                    DBG("Addfreev " << i << " " << realIdx % 12 << note.toString());
                    DBG("Addccccc " << i << " " << realIdx % 12 << note.toString());
                    break;
                }
                else if (note.ccMsg.getControllerValue() == channelToCcValue[realIdx])
                {
                    notesMidiSeq[i]->addEvent(note.noteMsg);

                    channelToNote[realIdx].insert(note.noteMsg.getNoteNumber());
                    noteToChannel[note.noteMsg.getNoteNumber()] = realIdx;

                    channelUseCount[realIdx] = channelUseCounter++;

                    needOverride = false;

                    DBG("Sameaddv " << i << " " << realIdx % 12 << note.toString());
                    break;
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

                channelUseCount[minIdx] = channelUseCounter++;

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
}

void MicroChromoAudioProcessor::sendAllNotesOff()
{
    synthBundle->sendAllNotesOff();
    psBundle->sendAllNotesOff();
    isPlayingNote = false;
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

    auto state = parameters.copyState();
    std::unique_ptr<XmlElement> xml = std::make_unique<XmlElement>("root");
    xml->setAttribute("numInstances", numInstancesParameter);
    xml->setAttribute("midiChannel", midiChannel);
    xml->setAttribute("ccBase", ccBase);
    xml->setAttribute("isModSrcKontakt", psModSource == USE_KONTAKT ? 1 : 0);

    auto stateXml = state.createXml();
    auto synthDescXml = synthBundle->getDescription().createXml();
    auto psDescXml = psBundle->getDescription().createXml();

    MemoryBlock synthData, psData;
    synthBundle->getStateInformation(synthData);
    psBundle->getStateInformation(psData);

    xml->addChildElement(stateXml.get());

    std::unique_ptr<XmlElement> synthXml, psXml;
    bool hasSynth = false, hasPs = false;
    if (synthBundle->isLoaded() && validDesc(synthBundle->getDescription()))
    {
        synthXml = std::make_unique<XmlElement>("synth_bundle");
        synthXml->addChildElement(synthDescXml.get());
        xml->addChildElement(synthXml.get());
        xml->setAttribute("synthStateData", synthData.toBase64Encoding());
        hasSynth = true;
    }

    if (psBundle->isLoaded() && validDesc(psBundle->getDescription()))
    {
        psXml = std::make_unique<XmlElement>("pitchshift_bundle");
        psXml->addChildElement(psDescXml.get());
        xml->addChildElement(psXml.get());
        xml->setAttribute("psStateData", psData.toBase64Encoding());
        hasPs = true;
    }

    copyXmlToBinary(*xml, destData);

    if (hasSynth)
        synthXml->removeChildElement(synthDescXml.get(), false);
    if (hasPs)
        psXml->removeChildElement(psDescXml.get(), false);

    xml->removeChildElement(stateXml.get(), false);
    xml->removeChildElement(synthXml.get(), false);
    xml->removeChildElement(psXml.get(), false);
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
            forEachXmlChildElement(*xml, child)
            {
                if (child->hasTagName(parameters.state.getType()))
                    parameters.replaceState(ValueTree::fromXml(*child));
                if (child->hasTagName("synth_bundle"))
                {
                    PluginDescription desc;
                    desc.loadFromXml(*child->getFirstChildElement());
                    if (validDesc(desc))
                    {
                        if (xml->hasAttribute("synthStateData"))
                        {
                            MemoryBlock proc_data;
                            proc_data.fromBase64Encoding(xml->getStringAttribute("synthStateData"));
                            if (xml->getIntAttribute("isModSrcKontakt", 0))
                            {
                                this->addPlugin(desc, true, [proc_data, this](PluginBundle& bundle) {
                                        bundle.setStateInformation(proc_data.getData(), proc_data.getSize());
                                        this->toggleUseKontakt(true);
                                    });
                            }
                            else
                                this->addPlugin(desc, true, [proc_data](PluginBundle& bundle) {bundle.setStateInformation(proc_data.getData(), proc_data.getSize()); });
                        }
                        else
                            this->addPlugin(desc, true);
                    }
                }
                if (child->hasTagName("pitchshift_bundle"))
                {
                    PluginDescription desc;
                    desc.loadFromXml(*child->getFirstChildElement());
                    if (validDesc(desc))
                    {
                        if (xml->hasAttribute("psStateData"))
                        {
                            MemoryBlock proc_data;
                            proc_data.fromBase64Encoding(xml->getStringAttribute("psStateData"));
                            this->addPlugin(desc, false, [proc_data](PluginBundle& bundle) {bundle.setStateInformation(proc_data.getData(), proc_data.getSize()); });
                        }
                        else
                            this->addPlugin(desc, false);
                    }
                }
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

        if (psBundle->getDescription().isDuplicateOf(this->internalTypes[2]))
            psBundle->setCcLearn(100, 0, 0.25f, 0.75f);

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

void MicroChromoAudioProcessor::updatePitchShiftModulationSource()
{
    updateModSrcSus = true;
    suspendProcessing(pluginLoadSus || updateModSrcSus || updateMidSeqSus);

    auto& synthCcLearnModule = synthBundle->getCcLearnModule();
    auto& psCcLearnModule = psBundle->getCcLearnModule();

    if (psModSource == USE_KONTAKT)
    {
        if (synthBundle->isKontakt())
        {
            if (synthCcLearnModule.hasLearned())
                synthCcLearnModule.reset();
            if (psCcLearnModule.hasLearned())
                psCcLearnModule.reset();
            ccBundle = synthBundle;
            if (!isCurrentModSrcKontakt)
                updateMidiSequence(jlimit(0, 116, ccBase.load()));
            DBG("Mod src now is: Kontakt");
        }
        else
        {
            AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, "Error", "Kontakt is not detected.");
            psModSource = USE_NONE;
            ccBundle = nullptr;
        }
    }
    else if (psModSource == USE_NONE)
    {
        if (synthCcLearnModule.hasLearned() && !psCcLearnModule.hasLearned())
        {
            psModSource = USE_SYNTH;
            ccBundle = synthBundle;
            if (isCurrentModSrcKontakt)
                updateMidiSequence(synthCcLearnModule.getCcSource());
            else
                updateCcMidiSequenceWithNewBase(synthCcLearnModule.getCcSource());
            DBG("Mod src now is: Synth " << ccBase);
        }
        else if (!synthCcLearnModule.hasLearned() && psCcLearnModule.hasLearned())
        {
            psModSource = USE_PS;
            ccBundle = psBundle;
            if (isCurrentModSrcKontakt)
                updateMidiSequence(psCcLearnModule.getCcSource());
            else
                updateCcMidiSequenceWithNewBase(psCcLearnModule.getCcSource());
            DBG("Mod src now is: Ps " << ccBase);
        }
        else if (synthCcLearnModule.hasLearned() && psCcLearnModule.hasLearned())
        {
            synthCcLearnModule.reset();
            psModSource = USE_PS;
            ccBundle = psBundle;
            if (isCurrentModSrcKontakt)
                updateMidiSequence(psCcLearnModule.getCcSource());
            else
                updateCcMidiSequenceWithNewBase(psCcLearnModule.getCcSource());
            DBG("Mod src now is: Ps " << ccBase);
        }
        else
        {
            psModSource = USE_NONE;
            ccBundle = nullptr;
            DBG("Mod src now is: None");
        }
    }
    else if (psModSource == USE_SYNTH)
    {
        if (!synthCcLearnModule.hasLearned())
        {
            psModSource = USE_NONE;
            ccBundle = nullptr;
            DBG("Mod src synth is None");
        }
        else
        {
            if (synthCcLearnModule.getCcSource() != ccBase)
            {
                updateCcMidiSequenceWithNewBase(synthCcLearnModule.getCcSource());
                DBG("Mod src update: Synth " << ccBase);
            }
            if (psCcLearnModule.hasLearned())
                psCcLearnModule.reset();
        }
    }
    else if (psModSource == USE_PS)
    {
        if (!psCcLearnModule.hasLearned())
        {
            psModSource = USE_NONE;
            ccBundle = nullptr;
            DBG("Mod src ps is None");
        }
        else
        {
            if (psCcLearnModule.getCcSource() != ccBase)
            {
                updateCcMidiSequenceWithNewBase(psCcLearnModule.getCcSource());
                DBG("Mod src update: Ps " << ccBase);
            }
            if (synthCcLearnModule.hasLearned())
                synthCcLearnModule.reset();
        }
    }

    updateModSrcSus = false;
    suspendProcessing(pluginLoadSus || updateModSrcSus || updateMidSeqSus);
}

bool MicroChromoAudioProcessor::canLearnCc(const PluginBundle* bundle)
{
    if (psModSource == USE_KONTAKT)
        return false;
    if (synthBundle.get() == bundle && psBundle->hasCcLearned())
        return false;
    if (psBundle.get() == bundle && synthBundle->hasCcLearned())
        return false;
    return true;
}

bool MicroChromoAudioProcessor::canChooseKontakt()
{
    if (!synthBundle->hasCcLearned() && !psBundle->hasCcLearned())
        return true;
    return false;
}

void MicroChromoAudioProcessor::toggleUseKontakt(bool isOn)
{
    if (isOn)
        psModSource = USE_KONTAKT;
    else
        psModSource = USE_NONE;
    updatePitchShiftModulationSource();
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

