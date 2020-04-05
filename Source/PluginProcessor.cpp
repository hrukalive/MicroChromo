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

    psBundle->setCcLearn(100, 0, 0.25f, 0.75f);
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

    auto sampleLength = 1.0 / getSampleRate();

    if (getPlayHead())
        getPlayHead()->getCurrentPosition(posInfo);
    else
        posInfo.isPlaying = false;
    MidiBuffer midiBuffer;
    if (!posInfo.isPlaying)
    {
        if (isPlayingNote)
        {
            midiMessages.clear();
            sendAllNotesOff(midiMessages);
        }
        else
        {
            MidiMessage midiMessage;
            int sampleOffset;
            for (MidiBuffer::Iterator it(midiMessages); it.getNextEvent(midiMessage, sampleOffset);)
            {
                midiMessage.setTimeStamp(sampleOffset * sampleLength + sampleLength);
                synthBundle->getCollectorAt(0)->addMessageToQueue(midiMessage);
            }
            midiMessages.clear();
        }
    }

    auto startTime = posInfo.timeInSeconds;
    auto bufferLength = buffer.getNumSamples() / getSampleRate();
    auto endTime = startTime + bufferLength;
    if (nextStartTime > 0.0 && std::abs(startTime - nextStartTime) > bufferLength)
        sendAllNotesOff(midiMessages);
    nextStartTime = endTime;
    if (isPlayingNote && startTime >= midiSeq.getEndTime())
        sendAllNotesOff(midiMessages);

    if (posInfo.isPlaying)
    {
        for (int i = midiSeq.getNextIndexAtTime(startTime); i < midiSeq.getNumEvents(); i++)
        {
            MidiMessageSequence::MidiEventHolder* event = midiSeq.getEventPointer(i);

            if (event->message.getTimeStamp() >= startTime && event->message.getTimeStamp() < endTime)
            {
                synthBundle->getCollectorAt(0)->addMessageToQueue(event->message);
                isPlayingNote = true;
            }
            if (event->message.getTimeStamp() >= endTime)
                break;
        }
    }

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
    synthBundle->adjustInstanceNumber(newNumInstances);
    psBundle->adjustInstanceNumber(newNumInstances, [newNumInstances, this]()
        {
            this->numInstancesParameter = newNumInstances;
        });
}

void MicroChromoAudioProcessor::updateMidiSequence(MidiMessageSequence seq)
{
    midiSeq.swapWith(seq);
    midiSeq.sort();
}

void MicroChromoAudioProcessor::sendAllNotesOff(MidiBuffer& midiMessages)
{
    synthBundle->sendAllNotesOff();
    psBundle->sendAllNotesOff();
    isPlayingNote = false;

    DBG("All note off");
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
        psBundle->loadPlugin(desc, numInstancesParameter, [desc, callback, this](PluginBundle& bundle)
            {
                if (callback)
                    callback(bundle);
                if (desc.isDuplicateOf(this->internalTypes[2]))
                    bundle.setCcLearn(100, 0, 0.25f, 0.75f);
            });
    }
}

void MicroChromoAudioProcessor::startLoadingPlugin()
{
    suspendProcessing(true);
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
        suspendProcessing(false);
    }
}


//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MicroChromoAudioProcessor();
}
