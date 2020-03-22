
#include <JuceHeader.h>
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

    void prepareToPlay(double newSampleRate, int) override {}

    void reset() override {}

    void releaseResources() override {}

    void processBlock(AudioBuffer<float>& buffer, MidiBuffer&) override {}

    using InternalPlugin::processBlock;
};

//==============================================================================
class SineWaveSynth : public InternalPlugin
{
    static AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        std::vector<std::unique_ptr<RangedAudioParameter>> params;

        params.push_back(std::make_unique<AudioParameterFloat>(
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
        params.push_back(std::make_unique<AudioParameterFloat>(
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
        params.push_back(std::make_unique<AudioParameterFloat>(
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
        params.push_back(std::make_unique<AudioParameterFloat>(
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
        params.push_back(std::make_unique<AudioParameterFloat>(
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

                            for (int i = outputBuffer.getNumChannels(); --i >= 0;)
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
    static AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        std::vector<std::unique_ptr<RangedAudioParameter>> params;

        params.push_back(std::make_unique<AudioParameterFloat>(
            "pitchshift",
            "Cent",
            NormalisableRange<float>(-1.0f,
                1.0f,
                0.01f),
            0.0f,
            String(),
            AudioProcessorParameter::genericParameter,
            [](const float value, int /*maximumStringLength*/)
            {
                return String(int(value * 100)) + " cents";
            },
            [](const String& text)
            {
                return text.getFloatValue() / 100.0f;
            }));
        params.push_back(std::make_unique<AudioParameterChoice>(
            "fftsize",
            "FFT Size",
            PitchShiftPlugin::fftSizes,
            3));
        params.push_back(std::make_unique<AudioParameterChoice>(
            "hopsize",
            "Hop Size",
            PitchShiftPlugin::hopSizes,
            0));
        params.push_back(std::make_unique<AudioParameterChoice>(
            "windowtype",
            "Window Type",
            PitchShiftPlugin::windowTypes,
            0));

        return { params.begin(), params.end() };
    }
public:
    PitchShiftPlugin(const PluginDescription& descr) : InternalPlugin(descr), 
        parameters(*this, nullptr, Identifier("Internal_PitchShift"), createParameterLayout())
    {
        parameters.addParameterListener("pitchshift", this);
        parameters.addParameterListener("fftsize", this);
        parameters.addParameterListener("hopsize", this);
        parameters.addParameterListener("windowtype", this);
        psParameter = parameters.getRawParameterValue("pitchshift");
        fftSizeParameter = parameters.getRawParameterValue("fftsize");
        hopSizeParameter = parameters.getRawParameterValue("hopsize");
        windowTypeParameter = parameters.getRawParameterValue("windowtype");
    }
    ~PitchShiftPlugin()
    {
        parameters.removeParameterListener("pitchshift", this);
        parameters.removeParameterListener("fftsize", this);
        parameters.removeParameterListener("hopsize", this);
        parameters.removeParameterListener("windowtype", this);
    }

    static String getIdentifier()
    {
        return "Pitch Shifter";
    }

    static PluginDescription getPluginDescription()
    {
        return InternalPlugin::getPluginDescription(getIdentifier(), false, false);
    }

    void parameterChanged(const String& parameterID, float newValue) override
    {
        if (parameterID == "pitchshift")
        {
            psValue.setTargetValue(newValue);
            return;
        }

        suspendProcessing(true);
        updateFftSize();
        updateHopSize();
        updateAnalysisWindow();
        updateWindowScaleFactor();
        //if (parameterID == "fftsize")
        //{
        //}
        //else if (parameterID == "hopsize")
        //{
        //}
        //else if (parameterID == "windowtype")
        //{
        //}
        suspendProcessing(false);
    }

    void prepareToPlay(double newSampleRate, int) override
    {
        psValue.reset(newSampleRate, 0.03);
        psValue.setCurrentAndTargetValue(*psParameter);

        updateFftSize();
        updateHopSize();
        updateAnalysisWindow();
        updateWindowScaleFactor();

        needToResetPhases = true;
    }

    void reset() override {}

    void releaseResources() override {}

    void processBlock(AudioBuffer<float>& buffer, MidiBuffer&) override
    {
        ScopedNoDenormals noDenormals;
        const int numInputChannels = getTotalNumInputChannels();
        const int numOutputChannels = getTotalNumOutputChannels();
        const int numSamples = buffer.getNumSamples();
        //======================================

        int currentInputBufferWritePosition;
        int currentOutputBufferWritePosition;
        int currentOutputBufferReadPosition;
        int currentSamplesSinceLastFFT;

        float shift = psValue.getNextValue();
        float ratio = roundf(shift * (float)hopSize) / (float)hopSize;
        int resampledLength = floorf((float)fftSize / ratio);
        HeapBlock<float> resampledOutput(resampledLength, true);
        HeapBlock<float> synthesisWindow(resampledLength, true);
        updateWindow(synthesisWindow, resampledLength);

        for (int channel = 0; channel < numInputChannels; ++channel) {
            float* channelData = buffer.getWritePointer(channel);

            currentInputBufferWritePosition = inputBufferWritePosition;
            currentOutputBufferWritePosition = outputBufferWritePosition;
            currentOutputBufferReadPosition = outputBufferReadPosition;
            currentSamplesSinceLastFFT = samplesSinceLastFFT;

            for (int sample = 0; sample < numSamples; ++sample) {

                //======================================

                const float in = channelData[sample];
                channelData[sample] = outputBuffer.getSample(channel, currentOutputBufferReadPosition);

                //======================================

                outputBuffer.setSample(channel, currentOutputBufferReadPosition, 0.0f);
                if (++currentOutputBufferReadPosition >= outputBufferLength)
                    currentOutputBufferReadPosition = 0;

                //======================================

                inputBuffer.setSample(channel, currentInputBufferWritePosition, in);
                if (++currentInputBufferWritePosition >= inputBufferLength)
                    currentInputBufferWritePosition = 0;

                //======================================

                if (++currentSamplesSinceLastFFT >= hopSize) {
                    currentSamplesSinceLastFFT = 0;

                    //======================================

                    int inputBufferIndex = currentInputBufferWritePosition;
                    for (int index = 0; index < fftSize; ++index) {
                        fftTimeDomain[index].real(sqrtf(fftWindow[index]) * inputBuffer.getSample(channel, inputBufferIndex));
                        fftTimeDomain[index].imag(0.0f);

                        if (++inputBufferIndex >= inputBufferLength)
                            inputBufferIndex = 0;
                    }

                    //======================================

                    fft->perform(fftTimeDomain, fftFrequencyDomain, false);

                    if (psValue.isSmoothing())
                        needToResetPhases = true;
                    if (shift == psValue.getTargetValue() && needToResetPhases) {
                        inputPhase.clear();
                        outputPhase.clear();
                        needToResetPhases = false;
                    }

                    for (int index = 0; index < fftSize; ++index) {
                        float magnitude = abs(fftFrequencyDomain[index]);
                        float phase = arg(fftFrequencyDomain[index]);

                        float phaseDeviation = phase - inputPhase.getSample(channel, index) - omega[index] * (float)hopSize;
                        float deltaPhi = omega[index] * hopSize + princArg(phaseDeviation);
                        float newPhase = princArg(outputPhase.getSample(channel, index) + deltaPhi * ratio);

                        inputPhase.setSample(channel, index, phase);
                        outputPhase.setSample(channel, index, newPhase);
                        fftFrequencyDomain[index] = std::polar(magnitude, newPhase);
                    }

                    fft->perform(fftFrequencyDomain, fftTimeDomain, true);

                    for (int index = 0; index < resampledLength; ++index) {
                        float x = (float)index * (float)fftSize / (float)resampledLength;
                        int ix = (int)floorf(x);
                        float dx = x - (float)ix;

                        float sample1 = fftTimeDomain[ix].real();
                        float sample2 = fftTimeDomain[(ix + 1) % fftSize].real();
                        resampledOutput[index] = sample1 + dx * (sample2 - sample1);
                        resampledOutput[index] *= sqrtf(synthesisWindow[index]);
                    }

                    //======================================

                    int outputBufferIndex = currentOutputBufferWritePosition;
                    for (int index = 0; index < resampledLength; ++index) {
                        float out = outputBuffer.getSample(channel, outputBufferIndex);
                        out += resampledOutput[index] * windowScaleFactor;
                        outputBuffer.setSample(channel, outputBufferIndex, out);

                        if (++outputBufferIndex >= outputBufferLength)
                            outputBufferIndex = 0;
                    }

                    //======================================

                    currentOutputBufferWritePosition += hopSize;
                    if (currentOutputBufferWritePosition >= outputBufferLength)
                        currentOutputBufferWritePosition = 0;
                }
            }
        }

        inputBufferWritePosition = currentInputBufferWritePosition;
        outputBufferWritePosition = currentOutputBufferWritePosition;
        outputBufferReadPosition = currentOutputBufferReadPosition;
        samplesSinceLastFFT = currentSamplesSinceLastFFT;

        for (int channel = numInputChannels; channel < numOutputChannels; ++channel)
            buffer.clear(channel, 0, numSamples);
    }

    using InternalPlugin::processBlock;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override
    {
        const auto state = parameters.copyState();
        const auto xml(state.createXml());
        copyXmlToBinary(*xml, destData);
    }
    void setStateInformation(const void* data, int sizeInBytes) override
    {
        std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

        if (xmlState != nullptr)
            if (xmlState->hasTagName(parameters.state.getType()))
                parameters.replaceState(ValueTree::fromXml(*xmlState));
    }

    //==============================================================================
    void updateFftSize()
    {
        fftSize = (int)*fftSizeParameter;
        fft = std::make_unique<dsp::FFT>(log2(fftSize));

        inputBufferLength = fftSize;
        inputBufferWritePosition = 0;
        inputBuffer.clear();
        inputBuffer.setSize(getTotalNumInputChannels(), inputBufferLength);

        float maxRatio = powf(2.0f, -1 / 12.0f);
        outputBufferLength = (int)floorf((float)fftSize / maxRatio);
        outputBufferWritePosition = 0;
        outputBufferReadPosition = 0;
        outputBuffer.clear();
        outputBuffer.setSize(getTotalNumInputChannels(), outputBufferLength);

        fftWindow.realloc(fftSize);
        fftWindow.clear(fftSize);

        fftTimeDomain.realloc(fftSize);
        fftTimeDomain.clear(fftSize);

        fftFrequencyDomain.realloc(fftSize);
        fftFrequencyDomain.clear(fftSize);

        samplesSinceLastFFT = 0;

        omega.realloc(fftSize);
        for (int index = 0; index < fftSize; ++index)
            omega[index] = MathConstants<float>::twoPi * index / (float)fftSize;

        inputPhase.clear();
        inputPhase.setSize(getTotalNumInputChannels(), outputBufferLength);

        outputPhase.clear();
        outputPhase.setSize(getTotalNumInputChannels(), outputBufferLength);
    }

    void updateHopSize()
    {
        overlap = (int)*hopSizeParameter;
        if (overlap != 0) {
            hopSize = fftSize / overlap;
            outputBufferWritePosition = hopSize % outputBufferLength;
        }
    }

    void updateAnalysisWindow()
    {
        updateWindow(fftWindow, fftSize);
    }

    void updateWindow(const HeapBlock<float>& window, const int windowLength)
    {
        switch ((int)*windowTypeParameter) {
        case windowTypeHann: {
            for (int sample = 0; sample < windowLength; ++sample)
                window[sample] = 0.5f - 0.5f * cosf(MathConstants<float>::twoPi * (float)sample / (float)(windowLength - 1));
            break;
        }
        case windowTypeHamming: {
            for (int sample = 0; sample < windowLength; ++sample)
                window[sample] = 0.54f - 0.46f * cosf(MathConstants<float>::twoPi * (float)sample / (float)(windowLength - 1));
            break;
        }
        case windowTypeBartlett: {
            for (int sample = 0; sample < windowLength; ++sample)
                window[sample] = 1.0f - fabs(2.0f * (float)sample / (float)(windowLength - 1) - 1.0f);
            break;
        }
        }
    }

    void updateWindowScaleFactor()
    {
        float windowSum = 0.0f;
        for (int sample = 0; sample < fftSize; ++sample)
            windowSum += fftWindow[sample];

        windowScaleFactor = 0.0f;
        if (overlap != 0 && windowSum != 0.0f)
            windowScaleFactor = 1.0f / (float)overlap / windowSum * (float)fftSize;
    }

    float princArg(const float phase)
    {
        if (phase >= 0.0f)
            return fmod(phase + MathConstants<float>::pi, MathConstants<float>::twoPi) - MathConstants<float>::pi;
        else
            return fmod(phase + MathConstants<float>::pi, -MathConstants<float>::twoPi) + MathConstants<float>::pi;
    }

private:
    inline static const StringArray fftSizes{ "32", "64", "128", "256", "512", "1024", "2048" };
    inline static const StringArray hopSizes{ "1/2 Overlap", "1/4 Overlap", "1/8 Overlap" };
    inline static const StringArray windowTypes{ "Hann", "Hamming", "Bartlett" };
    enum fftSizeIndex {
        fftSize32 = 0,
        fftSize64,
        fftSize128,
        fftSize256,
        fftSize512,
        fftSize1024,
        fftSize2048,
    };
    enum hopSizeIndex {
        hopSize2 = 0,
        hopSize4,
        hopSize8,
    };
    enum windowTypeIndex {
        windowTypeHann = 0,
        windowTypeHamming,
        windowTypeBartlett,
    };

    AudioProcessorValueTreeState parameters;
    SmoothedValue<float> psValue{ 0.0f };
    std::atomic<float>* psParameter{ nullptr }, * fftSizeParameter{ nullptr }, * hopSizeParameter{ nullptr }, * windowTypeParameter{ nullptr };

    int fftSize;
    std::unique_ptr<dsp::FFT> fft;

    int inputBufferLength;
    int inputBufferWritePosition;
    AudioSampleBuffer inputBuffer;

    int outputBufferLength;
    int outputBufferWritePosition;
    int outputBufferReadPosition;
    AudioSampleBuffer outputBuffer;

    HeapBlock<float> fftWindow;
    HeapBlock<dsp::Complex<float>> fftTimeDomain;
    HeapBlock<dsp::Complex<float>> fftFrequencyDomain;

    int samplesSinceLastFFT;

    int overlap;
    int hopSize;
    float windowScaleFactor;

    HeapBlock<float> omega;
    AudioSampleBuffer inputPhase;
    AudioSampleBuffer outputPhase;
    bool needToResetPhases;
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
