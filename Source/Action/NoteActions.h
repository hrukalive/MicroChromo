#pragma once

#include "Common.h"
#include "Note.h"

class Project;

//===----------------------------------------------------------------------===//
// Insert
//===----------------------------------------------------------------------===//

class NoteInsertAction final : public UndoableAction
{
public:
    explicit NoteInsertAction(Project& source) noexcept : source(source) {}
    NoteInsertAction(Project& source, const IdGenerator::Id trackId, const Note& target) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;

private:
    Project& source;
    IdGenerator::Id trackId;
    Note note;

    JUCE_DECLARE_NON_COPYABLE(NoteInsertAction)
};

//===----------------------------------------------------------------------===//
// Remove
//===----------------------------------------------------------------------===//

class NoteRemoveAction final : public UndoableAction
{
public:
    explicit NoteRemoveAction(Project& source) noexcept : source(source) {}

    NoteRemoveAction(Project& source,
        const String& trackId, const Note& target) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;

private:
    Project& source;
    IdGenerator::Id trackId;
    Note note;

    JUCE_DECLARE_NON_COPYABLE(NoteRemoveAction)
};

//===----------------------------------------------------------------------===//
// Change
//===----------------------------------------------------------------------===//

class NoteChangeAction final : public UndoableAction
{
public:
    explicit NoteChangeAction(Project& source) noexcept : source(source) {}

    NoteChangeAction(Project& source, const String& trackId,
        const Note& note, const Note& newParameters) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;
    UndoableAction* createCoalescedAction(UndoableAction* nextAction) override;

private:
    Project& source;
    IdGenerator::Id trackId;

    Note noteBefore;
    Note noteAfter;

    JUCE_DECLARE_NON_COPYABLE(NoteChangeAction)
};

//===----------------------------------------------------------------------===//
// Insert Group
//===----------------------------------------------------------------------===//

class NotesGroupInsertAction final : public UndoableAction
{
public:
    explicit NotesGroupInsertAction(Project& source) noexcept : source(source) {}

    NotesGroupInsertAction(Project& source,
        const String& trackId, Array<Note>& target) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;

private:
    Project& source;
    IdGenerator::Id trackId;
    Array<Note> notes;

    JUCE_DECLARE_NON_COPYABLE(NotesGroupInsertAction)
};

//===----------------------------------------------------------------------===//
// Remove Group
//===----------------------------------------------------------------------===//

class NotesGroupRemoveAction final : public UndoableAction
{
public:
    explicit NotesGroupRemoveAction(Project& source) noexcept : source(source) {}

    NotesGroupRemoveAction(Project& source,
        const String& trackId, Array<Note>& target) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;

private:
    Project& source;
    IdGenerator::Id trackId;
    Array<Note> notes;

    JUCE_DECLARE_NON_COPYABLE(NotesGroupRemoveAction)
};

//===----------------------------------------------------------------------===//
// Change Group
//===----------------------------------------------------------------------===//

class NotesGroupChangeAction final : public UndoableAction
{
public:

    explicit NotesGroupChangeAction(Project& source) noexcept : source(source) {}

    NotesGroupChangeAction(Project& source, const String& trackId,
        Array<Note>& state1, Array<Note>& state2) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;
    UndoableAction* createCoalescedAction(UndoableAction* nextAction) override;

private:
    Project& source;
    String trackId;

    Array<Note> notesBefore;
    Array<Note> notesAfter;

    JUCE_DECLARE_NON_COPYABLE(NotesGroupChangeAction)
};
