/*
  ==============================================================================

    TempoMarkerEvent.h
    Created: 12 Apr 2020 8:23:35pm
    Author:  bowen

  ==============================================================================
*/

#pragma once

#include "Common.h"
#include "MidiEvent.h"

#define TEMPO_DEFAULT_BPM 120.0f

class MidiTrack;

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

    void exportMessages(MidiMessageSequence& outSequence, double timeOffset, double timeFactor) const noexcept;

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
