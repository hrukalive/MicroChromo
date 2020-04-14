/*
  ==============================================================================

    TimeSignatureTrack.h
    Created: 13 Apr 2020 11:46:41am
    Author:  bowen

  ==============================================================================
*/

#pragma once

#include "Common.h"
#include "MidiTrack.h"
#include "TimeSignatureEvent.h"

class Project;

class TimeSignaturesTrack final : public MidiTrack
{
public:
    explicit TimeSignaturesTrack(Project& project) noexcept;

    //===------------------------------------------------------------------===//
    // Import/export
    //===------------------------------------------------------------------===//
    void importMidi(const MidiMessageSequence& sequence, short timeFormat) override;

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
    // Serializable
    //===------------------------------------------------------------------===//
    ValueTree serialize() const override;
    void deserialize(const ValueTree& tree) override;
    void reset() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimeSignaturesTrack);
};
