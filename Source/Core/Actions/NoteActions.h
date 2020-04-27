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

#include "Common.h"
#include "Note.h"

class Project;

//===----------------------------------------------------------------------===//
// Insert
//===----------------------------------------------------------------------===//
class NoteInsertAction final : public UndoableAction
{
public:
    explicit NoteInsertAction(Project& project) noexcept : project(project) {}
    NoteInsertAction(Project& project, const IdGenerator::Id trackId, const Note& target) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;

private:
    Project& project;
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
    explicit NoteRemoveAction(Project& project) noexcept : project(project) {}

    NoteRemoveAction(Project& project,
        const IdGenerator::Id trackId, const Note& target) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;

private:
    Project& project;
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
    explicit NoteChangeAction(Project& project) noexcept : project(project) {}

    NoteChangeAction(Project& project, const IdGenerator::Id trackId,
        const Note& note, const Note& newParameters) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;
    UndoableAction* createCoalescedAction(UndoableAction* nextAction) override;

private:
    Project& project;
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
    explicit NotesGroupInsertAction(Project& project) noexcept : project(project) {}

    NotesGroupInsertAction(Project& project,
        const IdGenerator::Id trackId, Array<Note>& target) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;

private:
    Project& project;
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
    explicit NotesGroupRemoveAction(Project& project) noexcept : project(project) {}

    NotesGroupRemoveAction(Project& project,
        const IdGenerator::Id trackId, Array<Note>& target) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;

private:
    Project& project;
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

    explicit NotesGroupChangeAction(Project& project) noexcept : project(project) {}

    NotesGroupChangeAction(Project& project, const IdGenerator::Id trackId,
        Array<Note>& state1, Array<Note>& state2) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;
    UndoableAction* createCoalescedAction(UndoableAction* nextAction) override;

private:
    Project& project;
    IdGenerator::Id trackId;

    Array<Note> notesBefore;
    Array<Note> notesAfter;

    JUCE_DECLARE_NON_COPYABLE(NotesGroupChangeAction)
};
