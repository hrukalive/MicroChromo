/*
  ==============================================================================

    VoiceScheduler.h
    Created: 23 Apr 2020 7:58:17pm
    Author:  bowen

  ==============================================================================
*/

#pragma once

#include "Common.h"

class Project;

class VoiceScheduler
{
public:
    VoiceScheduler(Project& owner);
    ~VoiceScheduler() = default;

    void schedule(OwnedArray<MidiMessageSequence>& noteSequences, OwnedArray<MidiMessageSequence>& ccSequences, 
        int n, int ccBase, bool kontaktMode, float ccTimeAdjustment, float timeMult = -1);

private:
    struct InternalMidiMessage
    {
        InternalMidiMessage() noexcept;
        InternalMidiMessage(int channel, int key, float timestamp, float velocity, int pitchbend, int cc, 
            bool isNoteOn, float adjustment);

        String toString() const;

        MidiMessage noteMsg, ccMsg;
        bool noteOn = false;

        static inline int compareElements(const InternalMidiMessage* const first, const InternalMidiMessage* const second)
        {
            InternalMidiMessage::compareElements(*first, *second);
        }

        static int compareElements(const InternalMidiMessage& first, const InternalMidiMessage& second);

        JUCE_LEAK_DETECTOR(InternalMidiMessage);
    };

    void scheduleKontakt(const Array<InternalMidiMessage>& sequence,
        OwnedArray<MidiMessageSequence>& noteSequences, OwnedArray<MidiMessageSequence>& ccSequences, int n);
    void scheduleGeneral(const Array<InternalMidiMessage>& sequence,
        OwnedArray<MidiMessageSequence>& noteSequences, OwnedArray<MidiMessageSequence>& ccSequences, int n);

    Project& project;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoiceScheduler);
};
