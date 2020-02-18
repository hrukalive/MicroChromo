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

	for (int i = 0; i < numInstances; i++)
	{
		synthArray.add(new PluginInstance(uid++, formatManager.createPluginInstance(internalTypes[0], 44100, 1024, String())));
		psArray.add(new PluginInstance(uid++, formatManager.createPluginInstance(internalTypes[0], 44100, 1024, String())));
	}
}

MicroChromoAudioProcessor::~MicroChromoAudioProcessor()
{
	synthArray.clear();
	psArray.clear();
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
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
	inputMeterSource.setMaxHoldMS(500);
	inputMeterSource.resize(getTotalNumInputChannels(), (int)(0.1 * sampleRate / samplesPerBlock));
	outputMeterSource.setMaxHoldMS(500);
	outputMeterSource.resize(getTotalNumOutputChannels(), (int)(0.1 * sampleRate / samplesPerBlock));

	bufferArray.clear();
	auto channels = jmax(synthArray[0]->processor->getTotalNumInputChannels(),
		synthArray[0]->processor->getTotalNumOutputChannels(),
		psArray[0]->processor->getTotalNumInputChannels(),
		psArray[0]->processor->getTotalNumOutputChannels());
	for (int i = 0; i < numInstances; i++)
	{
		bufferArray.add(new AudioBuffer<float>(channels, samplesPerBlock));
		//if (!synthArray[i]->getProcessor()->setBusesLayout(getBusesLayout()))
		//	AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, "Wrong", "Synth bus layout not supported.", "OK");
		//synthArray[i]->getProcessor()->enableAllBuses();
		//if (!psArray[i]->getProcessor()->setBusesLayout(getBusesLayout()))
		//	AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, "Wrong", "PS bus layout not supported.", "OK");
		//psArray[i]->getProcessor()->enableAllBuses();
		synthArray[i]->prepare(sampleRate, samplesPerBlock);
		psArray[i]->prepare(sampleRate, samplesPerBlock);
	}
}

void MicroChromoAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
	for (auto i = 0; i < numInstances; i++)
	{
		synthArray[i]->unprepare();
		psArray[i]->unprepare();
	}
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

	inputMeterSource.measureBlock(buffer);

	for (auto i = 0; i < buffer.getNumChannels(); i++)
	{
		for (auto j = 0; j < numInstances; j++)
		{
			bufferArray[j]->copyFrom(i, 0, buffer, i, 0, buffer.getNumSamples());
		}
	}

	for (auto i = 0; i < buffer.getNumChannels(); ++i)
		buffer.clear(i, 0, buffer.getNumSamples());

	for (auto i = 0; i < numInstances; i++)
	{
		if (synthArray[i] != nullptr && psArray[i] != nullptr)
		{
			synthArray[i]->processor->processBlock(*bufferArray[i], midiMessages);
			psArray[i]->processor->processBlock(*bufferArray[i], midiMessages);
			for (auto j = 0; j < buffer.getNumChannels(); j++)
			{
				buffer.addFrom(j, 0, *bufferArray[i], j, 0, buffer.getNumSamples());
			}
		}
	}
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

void MicroChromoAudioProcessor::addPlugin(const PluginDescription& desc, bool isSynth, GUICallback callback)
{
	instanceStarted = 0;
	suspendProcessing(true);
	for (auto i = 0; i < numInstances; i++)
	{
		formatManager.createPluginInstanceAsync(desc,
			getSampleRate(),
			getBlockSize(),
			[this, isSynth, i, callback](std::unique_ptr<AudioPluginInstance> instance, const String& error)
			{
				addPluginCallback(std::move(instance), error, isSynth, i);
				checkPluginLoaded(callback);
			});
	}
}

bool MicroChromoAudioProcessor::checkPluginLoaded(GUICallback callback)
{
	const ScopedLock lock(instanceIncrement);
	if (instanceStarted == numInstances)
	{
		if (callback != nullptr)
		{
			callback();
			prepareToPlay(getSampleRate(), getBlockSize());
		}
		suspendProcessing(false);
		return true;
	}
	return false;
}

void MicroChromoAudioProcessor::addPluginCallback(std::unique_ptr<AudioPluginInstance> instance, const String& error, bool isSynth, int index)
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
		if (isSynth)
			synthArray.set(index, new PluginInstance(uid++, std::move(instance)), true);
		else
			psArray.set(index, new PluginInstance(uid++, std::move(instance)), true);
		{
			const ScopedLock lock(instanceIncrement);
			instanceStarted++;
		}
	}
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MicroChromoAudioProcessor();
}
