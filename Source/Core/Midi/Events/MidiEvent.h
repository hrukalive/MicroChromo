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
#include "Serializable.h"
#include "SerializationKeys.h"

class MidiTrack;

//==============================================================================
class MidiEvent : public Serializable
{
public:
    enum class Type : uint8
    {
        Note = 1,
        TimeSignature = 2,
        TempoMarker = 3
    };

    inline Type getType() const noexcept { return this->type; }
    inline bool isTypeOf(Type val) const noexcept { return this->type == val; }

    MidiEvent(const MidiEvent& other) noexcept = default;
    MidiEvent& operator= (const MidiEvent & other) = default;

    MidiEvent(MidiEvent && other) noexcept = default;
    MidiEvent& operator= (MidiEvent && other) noexcept = default;

    MidiEvent(WeakReference<MidiTrack> owner, const MidiEvent& parameters) noexcept;
    MidiEvent(WeakReference<MidiTrack> owner, Type type, float beatVal) noexcept;

    //===------------------------------------------------------------------===//
    // Accessors
    //===------------------------------------------------------------------===//
    bool isValid() const noexcept;

    MidiTrack* getTrack() const noexcept;
    IdGenerator::Id getId() const noexcept;
    float getBeat() const noexcept;

    //===------------------------------------------------------------------===//
    // Helpers
    //===------------------------------------------------------------------===//
    static bool equalWithoutId(const MidiEvent* const first, const MidiEvent* const second) noexcept;
    static int compareElements(const MidiEvent* const first, const MidiEvent* const second) noexcept;

protected:
    WeakReference<MidiTrack> track;

    //==============================================================================
    IdGenerator::Id id;
    Type type;
    float beat;

private:
    JUCE_LEAK_DETECTOR(MidiEvent);
};
