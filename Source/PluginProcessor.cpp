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
			   std::make_unique<AudioParameterFloat>("gain", "Gain", 0.0f, 1.0f, 0.5f)
		   })
#endif
{
	PropertiesFile::Options options;
	options.folderName = "MicroChromo";
	options.applicationName = "MicroChromo Host";
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
}

MicroChromoAudioProcessor::~MicroChromoAudioProcessor()
{
	mainProcessor.clear();
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
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

	inputMeterSource.setMaxHoldMS(500);
	inputMeterSource.resize(getMainBusNumInputChannels(), (int)(0.1 * sampleRate / samplesPerBlock));
	outputMeterSource.setMaxHoldMS(500);
	outputMeterSource.resize(getMainBusNumOutputChannels(), (int)(0.1 * sampleRate / samplesPerBlock));

	mainProcessor.setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);
	mainProcessor.prepareToPlay(sampleRate, samplesPerBlock);
	initializeGraph();
}

void MicroChromoAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
	mainProcessor.releaseResources();
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

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    //for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    //    buffer.clear (i, 0, buffer.getNumSamples());

	//updateGraph();
	inputMeterSource.measureBlock(buffer);
	mainProcessor.processBlock(buffer, midiMessages);
	outputMeterSource.measureBlock(buffer);
}

//==============================================================================
bool MicroChromoAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* MicroChromoAudioProcessor::createEditor()
{
    return new MicroChromoAudioProcessorEditor (*this);
}

//==============================================================================
void MicroChromoAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
	auto state = parameters.copyState();
	std::unique_ptr<XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void MicroChromoAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
	std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(parameters.state.getType()))
			parameters.replaceState(ValueTree::fromXml(*xmlState));
}

void MicroChromoAudioProcessor::initializeGraph()
{
	mainProcessor.clear();

	audioInputNode = mainProcessor.addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::audioInputNode));
	audioOutputNode = mainProcessor.addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::audioOutputNode));
	midiInputNode = mainProcessor.addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::midiInputNode));
	midiOutputNode = mainProcessor.addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::midiOutputNode));

	connectAudioNodes();
	connectMidiNodes();
}

void MicroChromoAudioProcessor::connectAudioNodes()
{
	for (int channel = 0; channel < getMainBusNumOutputChannels(); ++channel)
		mainProcessor.addConnection({ { audioInputNode->nodeID,  channel },
										{ audioOutputNode->nodeID, channel } });
}

void MicroChromoAudioProcessor::connectMidiNodes()
{
	mainProcessor.addConnection({ { midiInputNode->nodeID,  AudioProcessorGraph::midiChannelIndex },
									{ midiOutputNode->nodeID, AudioProcessorGraph::midiChannelIndex } });
}

void MicroChromoAudioProcessor::updateGraph()
{
}

void MicroChromoAudioProcessor::addPlugin(const PluginDescription& desc, int slot_number, int copy_number, GUICallback callback)
{
	formatManager.createPluginInstanceAsync(desc,
		mainProcessor.getSampleRate(),
		mainProcessor.getBlockSize(),
		[this, slot_number, copy_number, callback](std::unique_ptr<AudioPluginInstance> instance, const String& error)
		{
			addPluginCallback(std::move(instance), error, slot_number, copy_number);
			callback();
		});
}

void MicroChromoAudioProcessor::addPluginCallback(std::unique_ptr<AudioPluginInstance> instance, const String& error, int slot_number, int copy_number)
{
	if (instance == nullptr)
	{
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
			TRANS("Couldn't create plugin"),
			error);
	}
	else
	{
		instance->enableAllBuses();

		if (auto node = mainProcessor.addNode(std::move(instance)))
		{
			node->properties.set("slot_number", slot_number);
			node->properties.set("copy_number", copy_number);
		}
	}
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MicroChromoAudioProcessor();
}
