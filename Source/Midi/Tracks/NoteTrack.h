#pragma once

#include "Common.h"
#include "MidiTrack.h"
#include "Note.h"

class Project;

class NoteTrack final : public MidiTrack
{
public:
    NoteTrack(Project& project) noexcept;
    NoteTrack(Project& project, const String& name, int channel) noexcept;

    //===------------------------------------------------------------------===//
    // Import/export
    //===------------------------------------------------------------------===//
    void importMidi(const MidiMessageSequence& sequence, short timeFormat) override;

    //===------------------------------------------------------------------===//
    // Undoable track editing
    //===------------------------------------------------------------------===//
    MidiEvent* insert(const Note& note, bool undoable);
    bool remove(const Note& note, bool undoable);
    bool change(const Note& note, const Note& newNote, bool undoable);

    bool insertGroup(Array<Note>& notes, bool undoable);
    bool removeGroup(Array<Note>& notes, bool undoable);
    bool changeGroup(Array<Note>& eventsBefore, Array<Note>& eventsAfter, bool undoable);

    //===------------------------------------------------------------------===//
    // Accessors
    //===------------------------------------------------------------------===//
    float getLastBeat() const noexcept override;

    //===------------------------------------------------------------------===//
    // Serializable
    //===------------------------------------------------------------------===//
    ValueTree serialize() const override;
    void deserialize(const ValueTree& tree) override;
    void reset() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoteTrack);
    JUCE_DECLARE_WEAK_REFERENCEABLE(NoteTrack);
};
