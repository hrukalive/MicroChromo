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
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ),
       parameters(*this, nullptr, Identifier("MicroChromoParam"),
           {
               std::make_unique<AudioParameterInt>("num_instances", "Num Inst", 1, 8, 1)
           })
#endif
{
    PropertiesFile::Options options;
    options.folderName = "MicroChromo";
    options.applicationName = "MicroChromo";
    options.filenameSuffix = "settings";
    options.osxLibrarySubFolder = "Preferences";
    appProperties.setStorageParameters(options);

    formatManager.addDefaultFormats();
    formatManager.addFormat(new InternalPluginFormat());
    InternalPluginFormat internalFormat;
    internalFormat.getAllTypes(internalTypes);
    if (auto savedPluginList = appProperties.getUserSettings()->getXmlValue("pluginList"))
        knownPluginList.recreateFromXml(*savedPluginList);
    for (auto& t : internalTypes)
        knownPluginList.addType(t);

    synthBundle.reset(new PluginBundle(8, internalTypes[1], *this));
    psBundle.reset(new PluginBundle(8, internalTypes[2], *this));

    synthBundle->loadPluginSync(numInstancesParameter);
    psBundle->loadPluginSync(numInstancesParameter);

    psBundle->setCcLearn(0, 0.25f, 0.75f);

    synthBundle->addChangeListener(this);
    psBundle->addChangeListener(this);
}

MicroChromoAudioProcessor::~MicroChromoAudioProcessor()
{
    psBundle->removeChangeListener(this);
    synthBundle = nullptr;
    psBundle = nullptr;
    bufferArray.clear();
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

void MicroChromoAudioProcessor::setCurrentProgram (int index)
{
}

const String MicroChromoAudioProcessor::getProgramName (int index)
{
    return {};
}

void MicroChromoAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void MicroChromoAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    inputMeterSource.setMaxHoldMS(500);
    inputMeterSource.resize(getTotalNumInputChannels(), (int)(0.1 * sampleRate / samplesPerBlock));
    outputMeterSource.setMaxHoldMS(500);
    outputMeterSource.resize(getTotalNumOutputChannels(), (int)(0.1 * sampleRate / samplesPerBlock));

    messageCollector.reset(sampleRate);

    bufferArray.clear();
    auto channels = jmax(synthBundle->getTotalNumInputChannels(),
        synthBundle->getTotalNumOutputChannels(),
        psBundle->getTotalNumInputChannels(),
        psBundle->getTotalNumOutputChannels());

    for (int i = 0; i < numInstancesParameter; i++)
        bufferArray.add(new AudioBuffer<float>(channels, samplesPerBlock));
    synthBundle->prepareToPlay(sampleRate, samplesPerBlock);
    psBundle->prepareToPlay(sampleRate, samplesPerBlock);
}

void MicroChromoAudioProcessor::releaseResources()
{
    synthBundle->releaseResources();
    psBundle->releaseResources();
    bufferArray.clear();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MicroChromoAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void MicroChromoAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    messageCollector.removeNextBlockOfMessages(midiMessages, getBlockSize());

    inputMeterSource.measureBlock(buffer);

    for (auto j = 0; j < numInstancesParameter; j++)
        bufferArray[j]->makeCopyOf(buffer);

    for (auto i = 0; i < buffer.getNumChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    synthBundle->processBlock(bufferArray, midiMessages);
    psBundle->processBlock(bufferArray, midiMessages);
    for (auto i = 0; i < numInstancesParameter; i++)
        for (auto j = 0; j < buffer.getNumChannels(); j++)
            buffer.addFrom(j, 0, *bufferArray[i], j, 0, buffer.getNumSamples());

    outputMeterSource.measureBlock(buffer);
}

void MicroChromoAudioProcessor::adjustInstanceNumber(int newNumInstances)
{
    synthBundle->adjustInstanceNumber(newNumInstances);
    psBundle->adjustInstanceNumber(newNumInstances, [newNumInstances, this]() {this->numInstancesParameter = newNumInstances; });
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
                            MemoryBlock data;
                            data.fromBase64Encoding(xml->getStringAttribute("synthStateData"));
                            this->addPlugin(desc, true, [data](PluginBundle& bundle) {bundle.setStateInformation(data.getData(), data.getSize()); });
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
                            MemoryBlock data;
                            data.fromBase64Encoding(xml->getStringAttribute("psStateData"));
                            this->addPlugin(desc, false, [data](PluginBundle& bundle) {bundle.setStateInformation(data.getData(), data.getSize()); });
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
    if (changed == synthBundle.get())
    {
        suspendProcessing(true);
        if (synthBundle->isLoaded())
        {
            prepareToPlay(getSampleRate(), getBlockSize());
            suspendProcessing(false);
        }
    }
    else if (changed == psBundle.get())
    {
        suspendProcessing(true);
        if (!psBundle->isLoading())
        {
            prepareToPlay(getSampleRate(), getBlockSize());
            suspendProcessing(false);
        }
    }
}

void MicroChromoAudioProcessor::addPlugin(const PluginDescription& desc, bool isSynth, std::function<void(PluginBundle&)> callback)
{
    if (isSynth)
    {
        synthBundle->setPluginDescription(desc);
        synthBundle->loadPlugin(numInstancesParameter, callback);
    }
    else
    {
        psBundle->setPluginDescription(desc);
        psBundle->loadPlugin(numInstancesParameter, [desc, callback, this](PluginBundle& bundle) 
            {
                if (desc.isDuplicateOf(this->internalTypes[2]))
                {
                    if (callback)
                        callback(bundle);
                    psBundle->setCcLearn(0, 0.25f, 0.75f);
                }
            });
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MicroChromoAudioProcessor();
}
