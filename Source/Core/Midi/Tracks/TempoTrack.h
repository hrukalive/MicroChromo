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
#include "TempoMarkerEvent.h"

class Project;

//==============================================================================
class TempoTrack final : public MidiTrack
{
public:
    explicit TempoTrack(Project& project) noexcept;

    //===------------------------------------------------------------------===//
    // Undoable track editing
    //===------------------------------------------------------------------===//
    MidiEvent* insert(const TempoMarkerEvent& signatureToCopy, bool undoable);
    bool remove(const TempoMarkerEvent& signature, bool undoable);
    bool change(const TempoMarkerEvent& signature,
        const TempoMarkerEvent& newSignature,
        bool undoable);

    bool insertGroup(Array<TempoMarkerEvent>& signatures, bool undoable);
    bool removeGroup(Array<TempoMarkerEvent>& signatures, bool undoable);
    bool changeGroup(Array<TempoMarkerEvent>& signaturesBefore,
        Array<TempoMarkerEvent>& signaturesAfter,
        bool undoable);

    //===------------------------------------------------------------------===//
    // Accessors
    //===------------------------------------------------------------------===//
    float getLastBeat() const noexcept override;

    inline TempoMarkerEvent* operator[] (int index) const noexcept
    {
        return dynamic_cast<TempoMarkerEvent*>(midiEvents[index]);
    }

    //===------------------------------------------------------------------===//
    // Serializable
    //===------------------------------------------------------------------===//
    ValueTree serialize() const override;
    void deserialize(const ValueTree& tree) override;
    void reset() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TempoTrack);
    JUCE_DECLARE_WEAK_REFERENCEABLE(TempoTrack);
};
