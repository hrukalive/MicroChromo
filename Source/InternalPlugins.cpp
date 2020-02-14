/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include <JuceHeader.h>
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

    void prepareToPlay (double newSampleRate, int) override
    {
    }

    void reset() override
    {
    }

    void releaseResources() override {}

    void processBlock (AudioBuffer<float>& buffer, MidiBuffer&) override
    {
    }

    using InternalPlugin::processBlock;
};

//==============================================================================
InternalPluginFormat::InternalPluginFormat()
{
}

std::unique_ptr<AudioPluginInstance> InternalPluginFormat::createInstance (const String& name)
{
    return std::make_unique<EmptyPlugin>(EmptyPlugin::getPluginDescription());
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
    results.add (EmptyPlugin::getPluginDescription());
}
