/*
  ==============================================================================

    MidiEvent.h
    Created: 12 Apr 2020 8:22:14pm
    Author:  bowen

  ==============================================================================
*/

#pragma once

#include "Common.h"
#include "Serializable.h"
#include "SerializationKeys.h"

class MidiTrack;

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

    virtual void exportMessages(MidiMessageSequence& outSequence, double timeOffset, double timeFactor) const noexcept = 0;

    //===------------------------------------------------------------------===//
    // Accessors
    //===------------------------------------------------------------------===//
    bool isValid() const noexcept;

    MidiTrack* getTrack() const noexcept;
    IdGenerator::Id getId() const noexcept;
    float getBeat() const noexcept;

    static int compareElements(const MidiEvent* const first, const MidiEvent* const second) noexcept;

protected:
    WeakReference<MidiTrack> track;

    IdGenerator::Id id;
    Type type;
    float beat;
};
