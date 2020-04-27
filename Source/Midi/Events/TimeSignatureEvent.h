/*
  ==============================================================================

    TimeSignatureEvent.h
    Created: 12 Apr 2020 8:23:29pm
    Author:  bowen

  ==============================================================================
*/

#pragma once

#include "Common.h"
#include "MidiEvent.h"

#define TIME_SIGNATURE_DEFAULT_NUMERATOR 4
#define TIME_SIGNATURE_DEFAULT_DENOMINATOR 4

class MidiTrack;

class TimeSignatureEvent final : public MidiEvent
{
public:
    TimeSignatureEvent() noexcept;

    TimeSignatureEvent(const TimeSignatureEvent& other) noexcept = default;
    TimeSignatureEvent& operator= (const TimeSignatureEvent& other) = default;

    TimeSignatureEvent(TimeSignatureEvent&& other) noexcept = default;
    TimeSignatureEvent& operator= (TimeSignatureEvent&& other) noexcept = default;

    TimeSignatureEvent(WeakReference<MidiTrack> owner, const TimeSignatureEvent& parametersToCopy) noexcept;
    explicit TimeSignatureEvent(WeakReference<MidiTrack> owner, float beatVal = 0.f,
        float numeratorVal = TIME_SIGNATURE_DEFAULT_NUMERATOR, 
        float denominatorVal = TIME_SIGNATURE_DEFAULT_DENOMINATOR) noexcept;

    void exportMessages(MidiMessageSequence& outSequence, double timeOffset, double timeFactor) const noexcept;

    TimeSignatureEvent withDeltaBeat(float beatOffset) const noexcept;
    TimeSignatureEvent withBeat(float newBeat) const noexcept;
    TimeSignatureEvent withNumerator(const int newNumerator) const noexcept;
    TimeSignatureEvent withDenominator(const int newDenominator) const noexcept;

    //===------------------------------------------------------------------===//
    // Accessors
    //===------------------------------------------------------------------===//

    int getNumerator() const noexcept;
    int getDenominator() const noexcept;

    //===------------------------------------------------------------------===//
    // Serializable
    //===------------------------------------------------------------------===//
    ValueTree serialize() const noexcept override;
    void deserialize(const ValueTree& tree) noexcept override;
    void reset() noexcept override;

    //===------------------------------------------------------------------===//
    // Helpers
    //===------------------------------------------------------------------===//

    void applyChanges(const TimeSignatureEvent& parameters) noexcept;

    static inline bool equalWithoutId(const MidiEvent* const first, const MidiEvent* const second) noexcept
    {
        return MidiEvent::equalWithoutId(first, second);
    }

    static inline bool equalWithoutId(const TimeSignatureEvent& first, const TimeSignatureEvent& second) noexcept
    {
        return TimeSignatureEvent::equalWithoutId(&first, &second);
    }

    static bool equalWithoutId(const TimeSignatureEvent* const first, const TimeSignatureEvent* const second) noexcept;

protected:
    int numerator = TIME_SIGNATURE_DEFAULT_NUMERATOR;
    int denominator = TIME_SIGNATURE_DEFAULT_DENOMINATOR;

private:
    JUCE_LEAK_DETECTOR(TimeSignatureEvent);
};