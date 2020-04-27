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

#define TIME_SIGNATURE_DEFAULT_NUMERATOR 4
#define TIME_SIGNATURE_DEFAULT_DENOMINATOR 4

class MidiTrack;

//==============================================================================
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