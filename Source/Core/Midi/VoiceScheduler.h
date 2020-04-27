/*
 * This file is part of the MicroChromo distribution
 * (https://github.com/hrukalive/MicroChromo).
 * Copyright (c) 2020 UIUC.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "Common.h"

class Project;

//==============================================================================
class VoiceScheduler
{
public:
    VoiceScheduler(Project& owner);
    ~VoiceScheduler() = default;

    //===------------------------------------------------------------------===//
    // Utility
    //===------------------------------------------------------------------===//
    void schedule(OwnedArray<MidiMessageSequence>& noteSequences, OwnedArray<MidiMessageSequence>& ccSequences, 
        int n, int ccBase, bool kontaktMode, float ccTimeAdjustment, float timeMult = -1);

private:
    struct InternalMidiMessage
    {
        InternalMidiMessage() noexcept;
        InternalMidiMessage(int channel, int key, float timestamp, float velocity, int pitchbend, int cc, 
            bool isNoteOn, float adjustment);

        //===------------------------------------------------------------------===//
        // Helpers
        //===------------------------------------------------------------------===//
        String toString() const;

        static inline int compareElements(const InternalMidiMessage* const first, const InternalMidiMessage* const second)
        {
            InternalMidiMessage::compareElements(*first, *second);
        }

        static int compareElements(const InternalMidiMessage& first, const InternalMidiMessage& second);

        MidiMessage noteMsg, ccMsg;
        bool noteOn = false;

        JUCE_LEAK_DETECTOR(InternalMidiMessage);
    };

    //==============================================================================
    void scheduleKontakt(const Array<InternalMidiMessage>& sequence,
        OwnedArray<MidiMessageSequence>& noteSequences, OwnedArray<MidiMessageSequence>& ccSequences, int n);
    void scheduleGeneral(const Array<InternalMidiMessage>& sequence,
        OwnedArray<MidiMessageSequence>& noteSequences, OwnedArray<MidiMessageSequence>& ccSequences, int n);

    //==============================================================================
    Project& project;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoiceScheduler);
};
