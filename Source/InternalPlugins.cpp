
#include "Common.h"
#include "SoundTouch.h"
#include "InternalPlugins.h"

//==============================================================================
class InternalPlugin   : public AudioPluginInstance
{
protected:
    InternalPlugin (const PluginDescription& descr,
                    const AudioChannelSet& channelSetToUse = AudioChannelSet::stereo())
        : AudioPluginInstance (getBusProperties (descr.numInputChannels == 0, channelSetToUse)),
          name  (descr.fileOrIdentifier.upToFirstOccurrenceOf (":", false, false)),
          state (descr.fileOrIdentifier.fromFirstOccurrenceOf (":", false, false)),
          isGenerator (descr.numInputChannels == 0),
          hasMidi (descr.isInstrument),
          channelSet (channelSetToUse)
    {
        jassert (channelSetToUse.size() == descr.numOutputChannels);
    }

public:
    //==============================================================================
    const String getName() const override                     { return name; }
    double getTailLengthSeconds() const override              { return 0.0; }
    bool acceptsMidi() const override                         { return hasMidi; }
    bool producesMidi() const override                        { return hasMidi; }
    AudioProcessorEditor* createEditor() override             { return nullptr; }
    bool hasEditor() const override                           { return false; }
    int getNumPrograms() override                             { return 0; }
    int getCurrentProgram() override                          { return 0; }
    void setCurrentProgram (int) override                     {}
    const String getProgramName (int) override                { return {}; }
    void changeProgramName (int, const String&) override      {}
    void getStateInformation (juce::MemoryBlock&) override    {}
    void setStateInformation (const void*, int) override      {}

    //==============================================================================
    bool isBusesLayoutSupported (const BusesLayout& layout) const override
    {
        if (! isGenerator)
            if (layout.getMainOutputChannelSet() != channelSet)
                return false;

        if (layout.getMainInputChannelSet() != channelSet)
            return false;

        return true;
    }

    //==============================================================================
    void fillInPluginDescription (PluginDescription& description) const override
    {
        description = getPluginDescription (name + ":" + state,
                                            isGenerator,
                                            hasMidi,
                                            channelSet);
    }

    static PluginDescription getPluginDescription (const String& identifier,
                                                   bool registerAsGenerator,
                                                   bool acceptsMidi,
                                                   const AudioChannelSet& channelSetToUse
                                                      = AudioChannelSet::stereo())
    {
        PluginDescription descr;
        auto pluginName  = identifier.upToFirstOccurrenceOf (":", false, false);
        auto pluginState = identifier.fromFirstOccurrenceOf (":", false, false);

        descr.name              = pluginName;
        descr.descriptiveName   = pluginName;
        descr.pluginFormatName  = "Internal";
        descr.category          = (registerAsGenerator ? (acceptsMidi ? "Synth" : "Generator") : "Effect");
        descr.manufacturerName  = "JUCE";
        descr.version           = ProjectInfo::versionString;
        descr.fileOrIdentifier  = pluginName + ":" + pluginState;
        descr.uid               = pluginName.hashCode();
        descr.isInstrument      = (acceptsMidi && registerAsGenerator);
        descr.numInputChannels  = (registerAsGenerator ? 0 : channelSetToUse.size());
        descr.numOutputChannels = channelSetToUse.size();

        return descr;
    }
private:
    static BusesProperties getBusProperties (bool registerAsGenerator,
                                             const AudioChannelSet& channelSetToUse)
    {
        return registerAsGenerator ? BusesProperties().withOutput ("Output", channelSetToUse)
                                   : BusesProperties().withInput  ("Input",  channelSetToUse)
                                                      .withOutput ("Output", channelSetToUse);
    }

    //==============================================================================
    String name, state;
    bool isGenerator, hasMidi;
    AudioChannelSet channelSet;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InternalPlugin)
};

//==============================================================================
class EmptyPlugin : public InternalPlugin
{
public:
    EmptyPlugin(const PluginDescription& descr) :   InternalPlugin (descr)
    {}

    static String getIdentifier()
    {
        return "Empty";
    }

    static PluginDescription getPluginDescription()
    {
        return InternalPlugin::getPluginDescription (getIdentifier(), false, false);
    }

    void prepareToPlay(double, int) override {}

    void reset() override {}

    void releaseResources() override {}

    void processBlock(AudioBuffer<float>&, MidiBuffer&) override {}

    using InternalPlugin::processBlock;
};

//==============================================================================
class SineWaveSynth : public InternalPlugin
{
    static AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        std::vector<std::unique_ptr<RangedAudioParameter>> params;

        params.push_back(std::make_unique<AudioParameterFloatVariant>(
            "outgain",
            "Out Gain",
            NormalisableRange<float>(0.0f,
                1.0f,
                0.0f,
                0.5f,
                false),
            0.5f,
            String(),
            AudioProcessorParameter::genericParameter,
            [](const float value, int /*maximumStringLength*/)
            {
                return String(Decibels::gainToDecibels(value), 2) +
                    " dB";
            },
            [](const String& text)
            {
                return Decibels::decibelsToGain(text.getFloatValue());
            }));
        params.push_back(std::make_unique<AudioParameterFloatVariant>(
            "attack",
            "Attack",
            NormalisableRange<float>(0.0f,
                2000.0f,
                0.0f,
                0.2f,
                false),
            1.0f,
            String(),
            AudioProcessorParameter::genericParameter,
            [](const float value, int /*maximumStringLength*/)
            {
                if (value >= 1000.0f)
                    return String(value / 1000.0f, 2) + " s";
                return String(value, 2) + " ms";
            },
            [](const String& text)
            {
                if (text.endsWith(" s") ||
                    (text.endsWith("s") && !text.endsWith("ms")))
                {
                    return text.getFloatValue() * 1000.0f;
                }
                return text.getFloatValue();
            }));
        params.push_back(std::make_unique<AudioParameterFloatVariant>(
            "decay",
            "Decay",
            NormalisableRange<float>(1.0f,
                2000.0f,
                0.0f,
                0.2f,
                false),
            600.0f,
            String(),
            AudioProcessorParameter::genericParameter,
            [](const float value, int /*maximumStringLength*/)
            {
                if (value >= 1000.0f)
                    return String(value / 1000.0f, 2) + " s";
                return String(value, 2) + " ms";
            },
            [](const String& text)
            {
                if (text.endsWith(" s") ||
                    (text.endsWith("s") && !text.endsWith("ms")))
                {
                    return text.getFloatValue() * 1000.0f;
                }
                return text.getFloatValue();
            }));
        params.push_back(std::make_unique<AudioParameterFloatVariant>(
            "sustain",
            "Sustain",
            NormalisableRange<float>(0.0f,
                1.0,
                0.0f,
                1.0f,
                false),
            1.0f,
            String(),
            AudioProcessorParameter::genericParameter,
            [](const float value, int /*maximumStringLength*/)
            {
                return String(Decibels::gainToDecibels(value), 2) +
                    " dB";
            },
            [](const String& text)
            {
                return Decibels::decibelsToGain(text.getFloatValue());
            }));
        params.push_back(std::make_unique<AudioParameterFloatVariant>(
            "release",
            "Release",
            NormalisableRange<float>(1.0f,
                2000.0f,
                0.0f,
                0.2f,
                false),
            50.0f,
            String(),
            AudioProcessorParameter::genericParameter,
            [](const float value, int /*maximumStringLength*/)
            {
                if (value >= 1000.0f)
                    return String(value / 1000.0f, 2) + " s";
                return String(value, 2) + " ms";
            },
            [](const String& text)
            {
                if (text.endsWith(" s") ||
                    (text.endsWith("s") && !text.endsWith("ms")))
                {
                    return text.getFloatValue() * 1000.0f;
                }
                return text.getFloatValue();
            }));
        return { params.begin(), params.end() };
    }

public:
    SineWaveSynth(const PluginDescription& descr) : InternalPlugin(descr), parameters(*this, nullptr, Identifier("Internal_Sine"), createParameterLayout())
    {
        gainParameter = parameters.getRawParameterValue("outgain");
        attackParameter = parameters.getRawParameterValue("attack");
        decayParameter = parameters.getRawParameterValue("decay");
        sustainParameter = parameters.getRawParameterValue("sustain");
        releaseParameter = parameters.getRawParameterValue("release");

        const int numVoices = 24;

        for (int i = numVoices; --i >= 0;)
            synth.addVoice(new SineWaveVoice(parameters, envParams));

        synth.addSound(new SineWaveSound());
    }

    static String getIdentifier()
    {
        return "Sine Wave Synth";
    }

    static PluginDescription getPluginDescription()
    {
        return InternalPlugin::getPluginDescription(getIdentifier(), true, true);
    }

    //==============================================================================
    void prepareToPlay(double newSampleRate, int) override
    {
        synth.setCurrentPlaybackSampleRate(newSampleRate);
    }

    void releaseResources() override {}

    //==============================================================================
    void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override
    {
        updateEnvParams();
        const int numSamples = buffer.getNumSamples();

        buffer.clear();
        synth.renderNextBlock(buffer, midiMessages, 0, numSamples);
        buffer.applyGain(*gainParameter);
    }

    using InternalPlugin::processBlock;

    void getStateInformation(juce::MemoryBlock& destData) override
    {
        auto xmlState = parameters.copyState();
        std::unique_ptr<XmlElement> xml(xmlState.createXml());
        copyXmlToBinary(*xml, destData);
    }
    void setStateInformation(const void* data, int sizeInBytes) override
    {
        std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

        if (xmlState.get() != nullptr)
            if (xmlState->hasTagName(parameters.state.getType()))
                parameters.replaceState(ValueTree::fromXml(*xmlState));
    }

private:

    void updateEnvParams()
    {
        envParams.attack = *attackParameter / 1000.0f;
        envParams.decay = *decayParameter / 1000.0f;
        envParams.sustain = *sustainParameter;
        envParams.release = *releaseParameter / 1000.0f;
    }

    //==============================================================================
    struct SineWaveSound : public SynthesiserSound
    {
        SineWaveSound() = default;

        bool appliesToNote(int /*midiNoteNumber*/) override { return true; }
        bool appliesToChannel(int /*midiChannel*/) override { return true; }
    };

    struct SineWaveVoice : public SynthesiserVoice
    {
        SineWaveVoice(AudioProcessorValueTreeState& apvts, ADSR::Parameters& envParams) :
            parameters(apvts),
            envParams(envParams)
        {
            envelope.setParameters(envParams);
        }

        bool canPlaySound(SynthesiserSound* sound) override
        {
            return dynamic_cast<SineWaveSound*> (sound) != nullptr;
        }

        void startNote(int midiNoteNumber, float velocity,
            SynthesiserSound* /*sound*/,
            int currentPitchWheelPosition) override
        {
            currentAngle = 0.0;
            level = velocity;

            currentNoteNumber = midiNoteNumber;
            pitchWheelMoved(currentPitchWheelPosition);

            updatePhaseIncrement();
            envelope.noteOn();
        }

        void stopNote(float /*velocity*/, bool allowTailOff) override
        {
            if (allowTailOff)
            {
                envelope.noteOff();
            }
            else
            {
                clearCurrentNote();
                angleDelta = 0.0;
                envelope.reset();
            }
        }

        void pitchWheelMoved(int newValue) override
        {
            if (newValue == 16383)
                pitchBend = 1.0f;
            else if (newValue == 8192)
                pitchBend = 0.0f;
            else if (newValue == 0)
                pitchBend = -1.0f;
            else
                pitchBend = ((newValue / 16383.0f) * 2.0f) - 1.0f;
            updatePhaseIncrement();
        }

        void controllerMoved(int /*controllerNumber*/, int /*newValue*/) override {}

        void updatePhaseIncrement()
        {
            const auto frequency = 440.0f * std::pow(2.0f, (currentNoteNumber + pitchBend - 69.0f) / 12.0f);
            double cyclesPerSample = frequency / getSampleRate();
            angleDelta = cyclesPerSample * 2.0 * double_Pi;
        }

        void renderNextBlock(AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
        {
            envelope.setParameters(envParams);
            if (angleDelta != 0.0)
            {
                while (--numSamples >= 0)
                {
                    if (envelope.isActive())
                    {
                        const auto envValue = envelope.getNextSample();
                        if (envValue != 0.0f)
                        {
                            const float currentSample = (float)(sin(currentAngle) * level * envValue);

                            for (int i = jmin(2, outputBuffer.getNumChannels()); --i >= 0;)
                                outputBuffer.addSample(i, startSample, currentSample);

                            currentAngle += angleDelta;
                            ++startSample;
                        }
                    }
                    else
                    {
                        clearCurrentNote();
                        angleDelta = 0.0;
                        envelope.reset();
                        break;
                    }
                }
            }
        }

        void setCurrentPlaybackSampleRate(double newRate)
        {
            SynthesiserVoice::setCurrentPlaybackSampleRate(newRate);
            if (newRate != 0)
                envelope.setSampleRate(newRate);
        }

        using SynthesiserVoice::renderNextBlock;

    private:
        AudioProcessorValueTreeState& parameters;
        ADSR::Parameters& envParams;
        ADSR envelope;

        double currentAngle = 0, angleDelta = 0, level = 0, pitchBend = 0;
        int currentNoteNumber = 60;
    };

    //==============================================================================
    std::atomic<float>* gainParameter{ nullptr };
    std::atomic<float>* attackParameter{ nullptr }, * decayParameter{ nullptr }, * sustainParameter{ nullptr }, * releaseParameter{ nullptr };
    AudioProcessorValueTreeState parameters;
    Synthesiser synth;
    ADSR::Parameters envParams;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SineWaveSynth)
};

//==============================================================================
class PitchShiftPlugin : public InternalPlugin, public AudioProcessorValueTreeState::Listener
{
    using SoundTouch = soundtouch::SoundTouch;

    static AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        std::vector<std::unique_ptr<RangedAudioParameter>> params;

        params.push_back(std::make_unique<AudioParameterFloatVariant>(
            "pitchshift",
            "Cent",
            NormalisableRange<float>(0.0f,
                1.0f,
                0.01f),
            0.5f,
            String(),
            AudioProcessorParameter::genericParameter,
            [](const float value, int /*maximumStringLength*/)
            {
                return String(int(value * 100) - 50) + " cents";
            },
            [](const String& text)
            {
                return (text.getFloatValue() + 50) / 100.0f;
            }));

        return { params.begin(), params.end() };
    }
public:
    PitchShiftPlugin(const PluginDescription& descr) : InternalPlugin(descr), 
        parameters(*this, nullptr, Identifier("Internal_PitchShift"), createParameterLayout())
    {
        parameters.addParameterListener("pitchshift", this);
        psParameter = parameters.getRawParameterValue("pitchshift");
    }
    ~PitchShiftPlugin()
    {
        parameters.removeParameterListener("pitchshift", this);
        shifters.clear();
    }

    static String getIdentifier()
    {
        return "Pitch Shifter";
    }

    static PluginDescription getPluginDescription()
    {
        return InternalPlugin::getPluginDescription(getIdentifier(), false, false);
    }

    void parameterChanged(const String& /*parameterID*/, float newValue) override
    {
        psValue.setTargetValue(newValue * 1 - 0.5);
    }

    void prepareToPlay(double sampleRate, int newBlockSize) override
    {
        psValue.reset(sampleRate, 0.05);
        psValue.setCurrentAndTargetValue(*psParameter);

        for (auto i = 0; i < getTotalNumInputChannels(); i++)
        {
            std::unique_ptr<SoundTouch> shifter{ new SoundTouch() };
            shifter->setSetting(SETTING_USE_AA_FILTER, 1);
            shifter->setChannels(1);
            shifter->setSampleRate((uint)sampleRate);
            shifter->adjustAmountOfSamples(newBlockSize);
            shifter->clear();
            shifters.add(std::move(shifter));
        }
    }

    void releaseResources() override
    {
        shifters.clear();
    }

    void processBlock(AudioBuffer<float>& buffer, MidiBuffer&) override
    {
        ScopedNoDenormals noDenormals;
        const int numInputChannels = getMainBusNumInputChannels();
        const int numSamples = buffer.getNumSamples();
        
        for (int i = 0; i < numSamples; i++)
        {
            for (int channel = 0; channel < numInputChannels; channel++)
            {
                shifters[channel]->setPitchSemiTones(psValue.getNextValue());
                shifters[channel]->putSamples(buffer.getReadPointer(channel, i), 1);
            }
        }
        for (int channel = 0; channel < numInputChannels; channel++)
            shifters[channel]->receiveSamples(buffer.getWritePointer(channel), numSamples);
        
        for (int channel = getMainBusNumOutputChannels(); channel < getTotalNumOutputChannels(); ++channel)
            buffer.clear(channel, 0, numSamples);
    }

    using InternalPlugin::processBlock;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override
    {
        const auto xmlState = parameters.copyState();
        const auto xml(xmlState.createXml());
        copyXmlToBinary(*xml, destData);
    }
    void setStateInformation(const void* data, int sizeInBytes) override
    {
        std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

        if (xmlState != nullptr)
            if (xmlState->hasTagName(parameters.state.getType()))
                parameters.replaceState(ValueTree::fromXml(*xmlState));
    }

private:
    AudioProcessorValueTreeState parameters;
    SmoothedValue<float> psValue{ 0.0f };
    std::atomic<float>* psParameter{ nullptr };

    OwnedArray<SoundTouch> shifters;
};

//==============================================================================
InternalPluginFormat::InternalPluginFormat() {}

std::unique_ptr<AudioPluginInstance> InternalPluginFormat::createInstance (const String& name)
{
    if (name == EmptyPlugin::getIdentifier()) return std::make_unique<EmptyPlugin>(EmptyPlugin::getPluginDescription());
    if (name == SineWaveSynth::getIdentifier()) return std::make_unique<SineWaveSynth>(SineWaveSynth::getPluginDescription());
    if (name == PitchShiftPlugin::getIdentifier()) return std::make_unique<PitchShiftPlugin>(PitchShiftPlugin::getPluginDescription());
    return {};
}

void InternalPluginFormat::createPluginInstance (const PluginDescription& desc,
                                                 double /*initialSampleRate*/, int /*initialBufferSize*/,
                                                 PluginCreationCallback callback)
{
    if (auto p = createInstance (desc.name))
        callback (std::move (p), {});
    else
        callback (nullptr, NEEDS_TRANS ("Invalid internal plugin name"));
}

bool InternalPluginFormat::requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const
{
    return false;
}

void InternalPluginFormat::getAllTypes (Array<PluginDescription>& results)
{
    results.add (EmptyPlugin::getPluginDescription(), SineWaveSynth::getPluginDescription(), PitchShiftPlugin::getPluginDescription());
}
