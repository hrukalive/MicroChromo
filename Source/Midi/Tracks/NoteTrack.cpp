/*
  ==============================================================================

    NoteTrack.cpp
    Created: 13 Apr 2020 11:46:34am
    Author:  bowen

  ==============================================================================
*/

#include "NoteTrack.h"
#include "Project.h"

NoteTrack::NoteTrack(Project& owner) noexcept :
    MidiTrack(owner) {}

//===------------------------------------------------------------------===//
// Import/export
//===------------------------------------------------------------------===//
void NoteTrack::importMidi(const MidiMessageSequence& sequence, short timeFormat)
{
    reset();
}

//===----------------------------------------------------------------------===//
// Undoable track editing
//===----------------------------------------------------------------------===//
MidiEvent* NoteTrack::insert(const Note& eventParams, const bool undoable)
{
    if (undoable)
    {
        this->getUndoStack()->
            perform(new NoteInsertAction(*this->getProject(),
                this->getTrackId(), eventParams));
    }
    else
    {
        const auto ownedNote = new Note(this, eventParams);
        midiEvents.addSorted(*ownedNote, ownedNote);
        project.broadcastAddEvent(*ownedNote);
        updateBeatRange(true);
        return ownedNote;
    }

    return nullptr;
}

bool NoteTrack::remove(const Note& eventParams, const bool undoable)
{
    if (undoable)
    {
        this->getUndoStack()->
            perform(new NoteRemoveAction(*this->getProject(),
                this->getTrackId(), eventParams));
    }
    else
    {
        const int index = this->midiEvents.indexOfSorted(eventParams, &eventParams);
        jassert(index >= 0);
        if (index >= 0)
        {
            auto* removedNote = this->midiEvents.getUnchecked(index);
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
        this->getUndoStack()->
            perform(new NoteChangeAction(*this->getProject(),
                this->getTrackId(), oldParams, newParams));
    }
    else
    {
        const int index = this->midiEvents.indexOfSorted(oldParams, &oldParams);
        jassert(index >= 0);
        if (index >= 0)
        {
            auto* changedNote = static_cast<Note*>(this->midiEvents.getUnchecked(index));
            changedNote->applyChanges(newParams);
            midiEvents.remove(index, false);
            midiEvents.addSorted(*changedNote, changedNote);
            project.broadcastChangeEvent(oldParams, *changedNote);
            updateBeatRange(true);
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
        this->getUndoStack()->
            perform(new NotesGroupInsertAction(*this->getProject(),
                this->getTrackId(), group));
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

        this->updateBeatRange(true);
    }

    return true;
}

bool NoteTrack::removeGroup(Array<Note>& group, bool undoable)
{
    if (undoable)
    {
        this->getUndoStack()->
            perform(new NotesGroupRemoveAction(*this->getProject(),
                this->getTrackId(), group));
    }
    else
    {
        for (int i = 0; i < group.size(); ++i)
        {
            const Note& note = group.getUnchecked(i);
            const int index = this->midiEvents.indexOfSorted(note, &note);

            jassert(index >= 0);
            if (index >= 0)
            {
                auto* removedNote = this->midiEvents.getUnchecked(index);
                project.broadcastRemoveEvent(*removedNote);
                midiEvents.remove(index, true);
            }
        }

        this->updateBeatRange(true);
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
        this->getUndoStack()->
            perform(new NotesGroupChangeAction(*this->getProject(),
                this->getTrackId(), groupBefore, groupAfter));
    }
    else
    {
        for (int i = 0; i < groupBefore.size(); ++i)
        {
            const Note& oldParams = groupBefore.getReference(i);
            const Note& newParams = groupAfter.getReference(i);
            const int index = this->midiEvents.indexOfSorted(oldParams, &oldParams);

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

        this->updateBeatRange(true);
    }

    return true;
}

//===----------------------------------------------------------------------===//
// Accessors
//===----------------------------------------------------------------------===//
float NoteTrack::getLastBeat() const noexcept
{
    float lastBeat = -FLT_MAX;
    if (this->midiEvents.size() == 0)
        return lastBeat;

    for (int i = 0; i < midiEvents.size(); i++)
    {
        const auto* n = static_cast<const Note*>(this->midiEvents.getUnchecked(i));
        lastBeat = jmax(lastBeat, n->getBeat() + n->getLength());
    }

    return lastBeat;
}


//===----------------------------------------------------------------------===//
// Serializable
//===----------------------------------------------------------------------===//
ValueTree NoteTrack::serialize() const
{
    ValueTree tree(Serialization::Midi::track);

    for (int i = 0; i < this->midiEvents.size(); ++i)
    {
        const MidiEvent* event = this->midiEvents.getUnchecked(i);
        tree.appendChild(event->serialize(), nullptr); // faster than addChildElement
    }

    return tree;
}

void NoteTrack::deserialize(const ValueTree& tree)
{
    reset();

    const auto root =
        tree.hasType(Serialization::Midi::track) ?
        tree : tree.getChildWithName(Serialization::Midi::track);

    if (!root.isValid())
        return;

    for (const auto& e : root)
    {
        if (e.hasType(Serialization::Midi::note))
        {
            auto note = new Note(this);
            note->deserialize(e);

            midiEvents.add(note);
        }
    }

    sort();
    updateBeatRange(false);
}

void NoteTrack::reset()
{
    midiEvents.clear();
}
