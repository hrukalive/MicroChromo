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

#define TEMPO_DEFAULT_BPM 120.0f

class MidiTrack;

//==============================================================================
class TempoMarkerEvent final : public MidiEvent
{
public:
    TempoMarkerEvent() noexcept;

    TempoMarkerEvent(const TempoMarkerEvent& other) noexcept = default;
    TempoMarkerEvent& operator= (const TempoMarkerEvent& other) = default;

    TempoMarkerEvent(TempoMarkerEvent&& other) noexcept = default;
    TempoMarkerEvent& operator= (TempoMarkerEvent&& other) noexcept = default;

    TempoMarkerEvent(WeakReference<MidiTrack> owner, const TempoMarkerEvent& parametersToCopy) noexcept;
    explicit TempoMarkerEvent(WeakReference<MidiTrack> owner, float beatVal = 0.f, float bpmVal = TEMPO_DEFAULT_BPM) noexcept;

    TempoMarkerEvent withDeltaBeat(float beatOffset) const noexcept;
    TempoMarkerEvent withBeat(float newBeat) const noexcept;
    TempoMarkerEvent withBPM(const float newBpm) const noexcept;

    //===------------------------------------------------------------------===//
    // Accessors
    //===------------------------------------------------------------------===//
    float getBPM() const noexcept;

    //===------------------------------------------------------------------===//
    // Serializable
    //===------------------------------------------------------------------===//
    ValueTree serialize() const noexcept override;
    void deserialize(const ValueTree& tree) noexcept override;
    void reset() noexcept override;

    //===------------------------------------------------------------------===//
    // Helpers
    //===------------------------------------------------------------------===//
    void applyChanges(const TempoMarkerEvent& parameters) noexcept;

    static inline bool equalWithoutId(const MidiEvent* const first, const MidiEvent* const second) noexcept
    {
        return MidiEvent::equalWithoutId(first, second);
    }

    static inline bool equalWithoutId(const TempoMarkerEvent& first, const TempoMarkerEvent& second) noexcept
    {
        return TempoMarkerEvent::equalWithoutId(&first, &second);
    }

    static bool equalWithoutId(const TempoMarkerEvent* const first, const TempoMarkerEvent* const second) noexcept;

protected:
    float bpm = TEMPO_DEFAULT_BPM;

private:
    JUCE_LEAK_DETECTOR(TempoMarkerEvent);
};
