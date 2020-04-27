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

#include "NoteTrack.h"

#include "NoteActions.h"
#include "Project.h"

//==============================================================================
NoteTrack::NoteTrack(Project& owner) noexcept :
    MidiTrack(owner) {}

NoteTrack::NoteTrack(Project& owner, const String& name, int channel) noexcept :
    MidiTrack(owner)
{
    this->name = name;
    this->channel = channel;
}

//===----------------------------------------------------------------------===//
// Undoable track editing
//===----------------------------------------------------------------------===//
MidiEvent* NoteTrack::insert(const Note& eventParams, const bool undoable)
{
    if (undoable)
    {
        project.getUndoManager().
            perform(new NoteInsertAction(project,
                getTrackId(), eventParams));
    }
    else
    {
        const auto ownedNote = new Note(this, eventParams);
        midiEvents.addSorted(*ownedNote, ownedNote);
        project.broadcastAddEvent(*ownedNote);
        updateBeatRange(true);
        project.broadcastPostAddEvent();
        return ownedNote;
    }

    return nullptr;
}

bool NoteTrack::remove(const Note& eventParams, const bool undoable)
{
    if (undoable)
    {
        project.getUndoManager().
            perform(new NoteRemoveAction(project,
                getTrackId(), eventParams));
    }
    else
    {
        const int index = midiEvents.indexOfSorted(eventParams, &eventParams);
        jassert(index >= 0);
        if (index >= 0)
        {
            auto* removedNote = midiEvents.getUnchecked(index);
            jassert(removedNote->isValid());
            project.broadcastRemoveEvent(*removedNote);
            midiEvents.remove(index, true);
            updateBeatRange(true);
            project.broadcastPostRemoveEvent(this);
            return true;
        }

        return false;
    }

    return true;
}

bool NoteTrack::change(const Note& oldParams,
    const Note& newParams, bool undoable)
{
    if (undoable)
    {
        project.getUndoManager().
            perform(new NoteChangeAction(project,
                getTrackId(), oldParams, newParams));
    }
    else
    {
        const int index = midiEvents.indexOfSorted(oldParams, &oldParams);
        jassert(index >= 0);
        if (index >= 0)
        {
            auto* changedNote = static_cast<Note*>(midiEvents.getUnchecked(index));
            changedNote->applyChanges(newParams);
            midiEvents.remove(index, false);
            midiEvents.addSorted(*changedNote, changedNote);
            project.broadcastChangeEvent(oldParams, *changedNote);
            updateBeatRange(true);
            project.broadcastPostChangeEvent();
            return true;
        }

        return false;
    }

    return true;
}

bool NoteTrack::insertGroup(Array<Note>& group, bool undoable)
{
    if (undoable)
    {
        project.getUndoManager().
            perform(new NotesGroupInsertAction(project,
                getTrackId(), group));
    }
    else
    {
        for (int i = 0; i < group.size(); ++i)
        {
            const Note& eventParams = group.getUnchecked(i);
            const auto ownedNote = new Note(this, eventParams);
            midiEvents.addSorted(*ownedNote, ownedNote);
            project.broadcastAddEvent(*ownedNote);
        }
        updateBeatRange(true);
        project.broadcastPostAddEvent();
    }

    return true;
}

bool NoteTrack::removeGroup(Array<Note>& group, bool undoable)
{
    if (undoable)
    {
        project.getUndoManager().
            perform(new NotesGroupRemoveAction(project,
                getTrackId(), group));
    }
    else
    {
        for (int i = 0; i < group.size(); ++i)
        {
            const Note& note = group.getUnchecked(i);
            const int index = midiEvents.indexOfSorted(note, &note);

            jassert(index >= 0);
            if (index >= 0)
            {
                auto* removedNote = midiEvents.getUnchecked(index);
                project.broadcastRemoveEvent(*removedNote);
                midiEvents.remove(index, true);
            }
        }
        updateBeatRange(true);
        project.broadcastPostRemoveEvent(this);
    }

    return true;
}

bool NoteTrack::changeGroup(Array<Note>& groupBefore,
    Array<Note>& groupAfter, bool undoable)
{
    jassert(groupBefore.size() == groupAfter.size());

    if (undoable)
    {
        project.getUndoManager().
            perform(new NotesGroupChangeAction(project,
                getTrackId(), groupBefore, groupAfter));
    }
    else
    {
        for (int i = 0; i < groupBefore.size(); ++i)
        {
            const Note& oldParams = groupBefore.getReference(i);
            const Note& newParams = groupAfter.getReference(i);
            const int index = midiEvents.indexOfSorted(oldParams, &oldParams);

            jassert(index >= 0);
            if (index >= 0)
            {
                auto* changedNote = static_cast<Note*>(midiEvents.getUnchecked(index));
                changedNote->applyChanges(newParams);
                midiEvents.remove(index, false);
                midiEvents.addSorted(*changedNote, changedNote);
                project.broadcastChangeEvent(oldParams, *changedNote);
            }
        }
        updateBeatRange(true);
        project.broadcastPostChangeEvent();
    }

    return true;
}

//===----------------------------------------------------------------------===//
// Accessors
//===----------------------------------------------------------------------===//
float NoteTrack::getLastBeat() const noexcept
{
    float lastBeat = -FLT_MAX;
    if (midiEvents.size() == 0)
        return lastBeat;

    for (int i = 0; i < midiEvents.size(); i++)
    {
        const auto* n = static_cast<const Note*>(midiEvents.getUnchecked(i));
        lastBeat = jmax(lastBeat, n->getBeat() + n->getLength());
    }

    return lastBeat;
}


//===----------------------------------------------------------------------===//
// Serializable
//===----------------------------------------------------------------------===//
ValueTree NoteTrack::serialize() const
{
    ValueTree tree(Serialization::Midi::notes);
    tree.setProperty(Serialization::Core::trackName, name, nullptr);
    tree.setProperty(Serialization::Core::trackChannel, channel, nullptr);
    for (int i = 0; i < midiEvents.size(); ++i)
    {
        const MidiEvent* event = midiEvents.getUnchecked(i);
        tree.appendChild(event->serialize(), nullptr); // faster than addChildElement
    }

    return tree;
}

void NoteTrack::deserialize(const ValueTree& tree)
{
    reset();

    const auto root =
        tree.hasType(Serialization::Midi::notes) ?
        tree : tree.getChildWithName(Serialization::Midi::notes);

    if (!root.isValid())
        return;

    if (root.hasProperty(Serialization::Core::trackId))
        id = root.getProperty(Serialization::Core::trackId, IdGenerator::generateId());
    name = root.getProperty(Serialization::Core::trackName, "empty");
    channel = root.getProperty(Serialization::Core::trackChannel, 1);
    for (const auto& e : root)
    {
        if (e.hasType(Serialization::Midi::note))
        {
            auto* note = new Note(this);
            note->deserialize(e);

            midiEvents.add(note);
        }
    }

    sort();
    updateBeatRange(false);
}

ValueTree NoteTrack::serializeWithId() const
{
    auto tree = serialize();
    tree.setProperty(Serialization::Core::trackId, id, nullptr);
    return tree;
}

void NoteTrack::reset()
{
    midiEvents.clear();
}
