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
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
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

    formatManager.addDefaultFormats();
    formatManager.addFormat(new InternalPluginFormat());
    InternalPluginFormat internalFormat;
    internalFormat.getAllTypes(internalTypes);
    if (auto savedPluginList = appProperties.getUserSettings()->getXmlValue("pluginList"))
        knownPluginList.recreateFromXml(*savedPluginList);
    parameterSlotNumber = jmax(appProperties.getUserSettings()->getIntValue("parameterSlotNumber", 16), 4);
    for (auto& t : internalTypes)
        knownPluginList.addType(t);

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

    synthBundle.reset(new PluginBundle(*this, MAX_INSTANCES, synthParamPtr));
    psBundle.reset(new PluginBundle(*this, MAX_INSTANCES, psParamPtr));

    synthBundle->loadPluginSync(internalTypes[1], numInstancesParameter);
    psBundle->loadPluginSync(internalTypes[2], numInstancesParameter);

    psBundle->setCcLearn(0, 0.25f, 0.75f);

    //synthBundle->addChangeListener(this);
    //psBundle->addChangeListener(this);
}

MicroChromoAudioProcessor::~MicroChromoAudioProcessor()
{
    synthParamPtr.clear();
    psParamPtr.clear();
    //synthBundle->removeChangeListener(this);
    //psBundle->removeChangeListener(this);
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
    //inputMeterSource.setMaxHoldMS(500);
    //inputMeterSource.resize(getTotalNumInputChannels(), (int)(0.1 * sampleRate / samplesPerBlock));
    //outputMeterSource.setMaxHoldMS(500);
    //outputMeterSource.resize(getTotalNumOutputChannels(), (int)(0.1 * sampleRate / samplesPerBlock));

    bufferArray.clear();
    for (int i = 0; i < numInstancesParameter; i++)
        bufferArray.add(new AudioBuffer<float>(synthBundle->getMainBusNumOutputChannels(), samplesPerBlock));

    synthBundle->prepareToPlay(sampleRate, samplesPerBlock);
    psBundle->prepareToPlay(sampleRate, samplesPerBlock);

    properlyPrepared = true;
}

void MicroChromoAudioProcessor::releaseResources()
{
    properlyPrepared = false;

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

    //inputMeterSource.measureBlock(buffer);

    for (auto j = 0; j < numInstancesParameter; j++)
        bufferArray[j]->makeCopyOf(buffer);

    for (auto i = 0; i < buffer.getNumChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    synthBundle->processBlock(bufferArray, midiMessages);
    psBundle->processBlock(bufferArray, midiMessages);
    for (auto i = 0; i < numInstancesParameter; i++)
        for (auto j = 0; j < buffer.getNumChannels(); j++)
            buffer.addFrom(j, 0, *bufferArray[i], j, 0, buffer.getNumSamples());
    midiMessages.clear();

    //outputMeterSource.measureBlock(buffer);

    if (getPlayHead())
        getPlayHead()->getCurrentPosition(info);
}

void MicroChromoAudioProcessor::adjustInstanceNumber(int newNumInstances)
{
    synthBundle->adjustInstanceNumber(newNumInstances, [newNumInstances, this]()
        {
        });
    psBundle->adjustInstanceNumber(newNumInstances, [newNumInstances, this]()
        {
            this->numInstancesParameter = newNumInstances;
        });
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
    //if (changed == synthBundle.get())
    //{
    //    suspendProcessing(true);
    //    if (synthBundle->isLoaded())
    //    {
    //        prepareToPlay(getSampleRate(), getBlockSize());
    //        suspendProcessing(false);
    //    }
    //    else if (synthBundle->isError())
    //        suspendProcessing(false);
    //}
    //else if (changed == psBundle.get())
    //{
    //    suspendProcessing(true);
    //    if (psBundle->isLoaded())
    //    {
    //        prepareToPlay(getSampleRate(), getBlockSize());
    //        suspendProcessing(false);
    //    }
    //    else if (synthBundle->isError())
    //        suspendProcessing(false);
    //}
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
                    bundle.setCcLearn(0, 0.25f, 0.75f);
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
            prepareToPlay(getSampleRate(), getBlockSize());
        suspendProcessing(false);
    }
}


//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MicroChromoAudioProcessor();
}
