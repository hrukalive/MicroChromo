/*
  ==============================================================================

    TempoTrack.h
    Created: 13 Apr 2020 11:46:46am
    Author:  bowen

  ==============================================================================
*/

#pragma once

#include "Common.h"
#include "MidiTrack.h"
#include "TempoMarkerEvent.h"

class Project;

class TempoTrack final : public MidiTrack
{
public:
    explicit TempoTrack(Project& project) noexcept;

    //===------------------------------------------------------------------===//
    // Import/export
    //===------------------------------------------------------------------===//
    void importMidi(const MidiMessageSequence& sequence, short timeFormat) override;

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
