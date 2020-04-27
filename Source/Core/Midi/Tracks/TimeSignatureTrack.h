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

#include "MidiTrack.h"
#include "TimeSignatureEvent.h"

class Project;

//==============================================================================
class TimeSignatureTrack final : public MidiTrack
{
public:
    explicit TimeSignatureTrack(Project& project) noexcept;

    //===------------------------------------------------------------------===//
    // Undoable track editing
    //===------------------------------------------------------------------===//
    MidiEvent* insert(const TimeSignatureEvent& signatureToCopy, bool undoable);
    bool remove(const TimeSignatureEvent& signature, bool undoable);
    bool change(const TimeSignatureEvent& signature,
        const TimeSignatureEvent& newSignature,
        bool undoable);

    bool insertGroup(Array<TimeSignatureEvent>& signatures, bool undoable);
    bool removeGroup(Array<TimeSignatureEvent>& signatures, bool undoable);
    bool changeGroup(Array<TimeSignatureEvent>& signaturesBefore,
        Array<TimeSignatureEvent>& signaturesAfter,
        bool undoable);

    //===------------------------------------------------------------------===//
    // Accessors
    //===------------------------------------------------------------------===//
    float getLastBeat() const noexcept override;

    inline TimeSignatureEvent* operator[] (int index) const noexcept
    {
        return dynamic_cast<TimeSignatureEvent*>(midiEvents[index]);
    }

    //===------------------------------------------------------------------===//
    // Serializable
    //===------------------------------------------------------------------===//
    ValueTree serialize() const override;
    void deserialize(const ValueTree& tree) override;
    void reset() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimeSignatureTrack);
    JUCE_DECLARE_WEAK_REFERENCEABLE(TimeSignatureTrack);
};
