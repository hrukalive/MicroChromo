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

#include "NoteActions.h"

#include "SerializationKeys.h"
#include "Project.h"
#include "NoteTrack.h"

//===----------------------------------------------------------------------===//
// Insert
//===----------------------------------------------------------------------===//
NoteInsertAction::NoteInsertAction(Project& project,
    IdGenerator::Id trackId, const Note& event) noexcept :
    project(project),
    trackId(trackId),
    note(event) {}

bool NoteInsertAction::perform()
{
    if (NoteTrack* track = project.findTrackById<NoteTrack>(trackId))
    {
        return (track->insert(note, false) != nullptr);
    }

    return false;
}

bool NoteInsertAction::undo()
{
    if (NoteTrack* track = project.findTrackById<NoteTrack>(trackId))
    {
        return track->remove(note, false);
    }

    return false;
}

int NoteInsertAction::getSizeInUnits()
{
    return sizeof(Note);
}

//===----------------------------------------------------------------------===//
// Remove
//===----------------------------------------------------------------------===//
NoteRemoveAction::NoteRemoveAction(Project& project,
    IdGenerator::Id trackId, const Note& event) noexcept :
    project(project),
    trackId(trackId),
    note(event) {}

bool NoteRemoveAction::perform()
{
    if (NoteTrack* track = project.findTrackById<NoteTrack>(trackId))
    {
        return track->remove(note, false);
    }

    return false;
}

bool NoteRemoveAction::undo()
{
    if (NoteTrack* track = project.findTrackById<NoteTrack>(trackId))
    {
        return (track->insert(note, false) != nullptr);
    }

    return false;
}

int NoteRemoveAction::getSizeInUnits()
{
    return sizeof(Note);
}

//===----------------------------------------------------------------------===//
// Change
//===----------------------------------------------------------------------===//
NoteChangeAction::NoteChangeAction(Project& project,
    IdGenerator::Id trackId, const Note& note, const Note& newParameters) noexcept :
    project(project),
    trackId(trackId),
    noteBefore(note),
    noteAfter(newParameters)
{
    jassert(note.getId() == newParameters.getId());
}

bool NoteChangeAction::perform()
{
    if (NoteTrack* track = project.findTrackById<NoteTrack>(trackId))
    {
        return track->change(noteBefore, noteAfter, false);
    }

    return false;
}

bool NoteChangeAction::undo()
{
    if (NoteTrack* track = project.findTrackById<NoteTrack>(trackId))
    {
        return track->change(noteAfter, noteBefore, false);
    }

    return false;
}

int NoteChangeAction::getSizeInUnits()
{
    return sizeof(Note) * 2;
}

UndoableAction* NoteChangeAction::createCoalescedAction(UndoableAction* nextAction)
{
    if (NoteTrack* track = project.findTrackById<NoteTrack>(trackId))
    {
        if (NoteChangeAction* nextChanger =
            dynamic_cast<NoteChangeAction*>(nextAction))
        {
            const bool idsAreEqual =
                (noteBefore.getId() == nextChanger->noteAfter.getId() &&
                    trackId == nextChanger->trackId);

            if (idsAreEqual)
            {
                return new NoteChangeAction(project,
                    trackId, noteBefore, nextChanger->noteAfter);
            }
        }
    }

    (void)nextAction;
    return nullptr;
}

//===----------------------------------------------------------------------===//
// Insert Group
//===----------------------------------------------------------------------===//
NotesGroupInsertAction::NotesGroupInsertAction(Project& project,
    IdGenerator::Id trackId, Array<Note>& target) noexcept :
    project(project),
    trackId(trackId)
{
    notes.swapWith(target);
}

bool NotesGroupInsertAction::perform()
{
    if (NoteTrack* track = project.findTrackById<NoteTrack>(trackId))
    {
        return track->insertGroup(notes, false);
    }

    return false;
}

bool NotesGroupInsertAction::undo()
{
    if (NoteTrack* track = project.findTrackById<NoteTrack>(trackId))
    {
        return track->removeGroup(notes, false);
    }

    return false;
}

int NotesGroupInsertAction::getSizeInUnits()
{
    return (sizeof(Note) * notes.size());
}

//===----------------------------------------------------------------------===//
// Remove Group
//===----------------------------------------------------------------------===//
NotesGroupRemoveAction::NotesGroupRemoveAction(Project& project,
    IdGenerator::Id trackId, Array<Note>& target) noexcept :
    project(project),
    trackId(trackId)
{
    notes.swapWith(target);
}

bool NotesGroupRemoveAction::perform()
{
    if (NoteTrack* track = project.findTrackById<NoteTrack>(trackId))
    {
        return track->removeGroup(notes, false);
    }

    return false;
}

bool NotesGroupRemoveAction::undo()
{
    if (NoteTrack* track = project.findTrackById<NoteTrack>(trackId))
    {
        return track->insertGroup(notes, false);
    }

    return false;
}

int NotesGroupRemoveAction::getSizeInUnits()
{
    return (sizeof(Note) * notes.size());
}

//===----------------------------------------------------------------------===//
// Change Group
//===----------------------------------------------------------------------===//
NotesGroupChangeAction::NotesGroupChangeAction(Project& project,
    IdGenerator::Id trackId, Array<Note>& state1, Array<Note>& state2) noexcept :
    project(project),
    trackId(trackId)
{
    notesBefore.swapWith(state1);
    notesAfter.swapWith(state2);
}

bool NotesGroupChangeAction::perform()
{
    if (NoteTrack* track = project.findTrackById<NoteTrack>(trackId))
    {
        return track->changeGroup(notesBefore, notesAfter, false);
    }

    return false;
}

bool NotesGroupChangeAction::undo()
{
    if (NoteTrack* track = project.findTrackById<NoteTrack>(trackId))
    {
        return track->changeGroup(notesAfter, notesBefore, false);
    }

    return false;
}

int NotesGroupChangeAction::getSizeInUnits()
{
    return (sizeof(Note) * notesBefore.size()) +
        (sizeof(Note) * notesAfter.size());
}

UndoableAction* NotesGroupChangeAction::createCoalescedAction(UndoableAction* nextAction)
{
    if (NoteTrack* track = project.findTrackById<NoteTrack>(trackId))
    {
        if (NotesGroupChangeAction* nextChanger =
            dynamic_cast<NotesGroupChangeAction*>(nextAction))
        {
            if (nextChanger->trackId != trackId)
            {
                return nullptr;
            }

            if (notesBefore.size() != nextChanger->notesAfter.size())
            {
                return nullptr;
            }

            for (int i = 0; i < notesBefore.size(); ++i)
            {
                if (notesBefore.getUnchecked(i).getId() !=
                    nextChanger->notesAfter.getUnchecked(i).getId())
                {
                    return nullptr;
                }
            }

            return new NotesGroupChangeAction(project,
                trackId, notesBefore, nextChanger->notesAfter);
        }
    }

    (void)nextAction;
    return nullptr;
}
