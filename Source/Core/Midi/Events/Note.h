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

#include "MidiEvent.h"

#define MIDDLE_C 60

class MidiTrack;

//==============================================================================
class Note final : public MidiEvent
{
public:

    Note() noexcept;

    Note(const Note &other) noexcept = default;
    Note &operator= (const Note &other) = default;

    Note(Note &&other) noexcept = default;
    Note &operator= (Note &&other) noexcept = default;

    Note(WeakReference<MidiTrack> owner, const Note& parametersToCopy) noexcept;
    explicit Note(WeakReference<MidiTrack> owner, int keyVal = MIDDLE_C, float beatVal = 0.f,
         float lengthVal = 1.f, float velocityVal = 1.f, String pitchColorVal = "0") noexcept;

    Note withKey(int newKey) const noexcept;
    Note withDeltaKey(int deltaKey) const noexcept;
    Note withBeat(float newBeat) const noexcept;
    Note withDeltaBeat(float deltaPosition) const noexcept;
    Note withKeyBeat(int newKey, float newBeat) const noexcept;
    Note withKeyLength(int newKey, float newLength) const noexcept;
    Note withLength(float newLength) const noexcept;
    Note withDeltaLength(float deltaLength) const noexcept;
    Note withVelocity(float newVelocity) const noexcept;
    Note withPitchColor(String pitchColor) const noexcept;

    //===------------------------------------------------------------------===//
    // Accessors
    //===------------------------------------------------------------------===//
    int getKey() const noexcept;
    float getLength() const noexcept;
    float getVelocity() const noexcept;
    String getPitchColor() const noexcept;

    void setKey(int newKey) noexcept;
    void setBeat(float newBeat) noexcept;
    void setLength(float newLength) noexcept;
    void setVelocity(float newVelocity) noexcept;
    void setPitchColor(String newPitchColor) noexcept;

    //===------------------------------------------------------------------===//
    // Serializable
    //===------------------------------------------------------------------===//
    ValueTree serialize() const noexcept override;
    void deserialize(const ValueTree& tree) noexcept override;
    void reset() noexcept override;

    //===------------------------------------------------------------------===//
    // Helpers
    //===------------------------------------------------------------------===//
    void applyChanges(const Note& parameters) noexcept;

    static inline bool equalWithoutId(const MidiEvent* const first, const MidiEvent* const second) noexcept
    {
        return MidiEvent::equalWithoutId(first, second);
    }

    static inline bool equalWithoutId(const Note& first, const Note& second) noexcept
    {
        return Note::equalWithoutId(&first, &second);
    }

    static bool equalWithoutId(const Note* const first, const Note* const second) noexcept;

    static inline int compareElements(const MidiEvent* const first, const MidiEvent* const second) noexcept
    {
        return MidiEvent::compareElements(first, second);
    }

    static inline int compareElements(const Note& first, const Note& second) noexcept
    {
        return Note::compareElements(&first, &second);
    }

    static int compareElements(const Note* const first, const Note* const second) noexcept;

protected:
    int key = MIDDLE_C;
    float length = 1.f;
    float velocity = 1.f;
    String pitchColor = "+0";

private:
    JUCE_LEAK_DETECTOR(Note);
};

struct NoteComparator
{
    NoteComparator() = default;

    static int NoteComparator::compareElements(const Note& first, const Note& second)
    {
        const float timeDiff = first.getBeat() - second.getBeat();
        return (timeDiff > 0.f) - (timeDiff < 0.f);
    }
};

