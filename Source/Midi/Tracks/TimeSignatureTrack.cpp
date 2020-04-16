/*
  ==============================================================================

    TimeSignatureTrack.cpp
    Created: 13 Apr 2020 11:46:41am
    Author:  bowen

  ==============================================================================
*/

#include "TimeSignatureTrack.h"
#include "TimeSignatureEventActions.h"
#include "Project.h"

TimeSignatureTrack::TimeSignatureTrack(Project& project) noexcept :
    MidiTrack(project) {}

//===----------------------------------------------------------------------===//
// Import/export
//===----------------------------------------------------------------------===//

void TimeSignatureTrack::importMidi(const MidiMessageSequence& sequence, short timeFormat)
{
    reset();
    for (int i = 0; i < sequence.getNumEvents(); ++i)
    {
        const MidiMessage& message = sequence.getEventPointer(i)->message;
        if (message.isTimeSignatureMetaEvent())
        {
            int numerator = 0;
            int denominator = 0;
            message.getTimeSignatureInfo(numerator, denominator);
            const float startBeat = MidiTrack::midiTicksToBeats(message.getTimeStamp(), timeFormat);
            const TimeSignatureEvent signature(this, startBeat, numerator, denominator);
            importMidiEvent<TimeSignatureEvent>(signature);
        }
    }

    updateBeatRange(false);
}

//===----------------------------------------------------------------------===//
// Undoable track editing
//===----------------------------------------------------------------------===//

MidiEvent* TimeSignatureTrack::insert(const TimeSignatureEvent& eventParams, bool undoable)
{
    if (undoable)
    {
        project.getUndoManager().
            perform(new TimeSignatureEventInsertAction(project,
                getTrackId(), eventParams));
    }
    else
    {
        auto* ownedEvent = new TimeSignatureEvent(this, eventParams);
        midiEvents.addSorted(*ownedEvent, ownedEvent);
        project.broadcastAddEvent(*ownedEvent);
        updateBeatRange(true);
        project.broadcastPostAddEvent();
        return ownedEvent;
    }

    return nullptr;
}

bool TimeSignatureTrack::remove(const TimeSignatureEvent& signature, bool undoable)
{
    if (undoable)
    {
        project.getUndoManager().
            perform(new TimeSignatureEventRemoveAction(project,
                getTrackId(), signature));
    }
    else
    {
        const int index = midiEvents.indexOfSorted(signature, &signature);
        if (index >= 0)
        {
            auto* removedEvent = midiEvents.getUnchecked(index);
            project.broadcastRemoveEvent(*removedEvent);
            midiEvents.remove(index, true);
            updateBeatRange(true);
            project.broadcastPostRemoveEvent(this);
            return true;
        }

        return false;
    }

    return true;
}

bool TimeSignatureTrack::change(const TimeSignatureEvent& oldParams,
    const TimeSignatureEvent& newParams, bool undoable)
{
    if (undoable)
    {
        project.getUndoManager().
            perform(new TimeSignatureEventChangeAction(project,
                getTrackId(), oldParams, newParams));
    }
    else
    {
        const int index = midiEvents.indexOfSorted(oldParams, &oldParams);
        if (index >= 0)
        {
            auto* changedEvent = static_cast<TimeSignatureEvent*>(midiEvents.getUnchecked(index));
            changedEvent->applyChanges(newParams);
            midiEvents.remove(index, false);
            midiEvents.addSorted(*changedEvent, changedEvent);
            project.broadcastChangeEvent(oldParams, *changedEvent);
            updateBeatRange(true);
            project.broadcastPostChangeEvent();
            return true;
        }

        return false;
    }

    return true;
}

bool TimeSignatureTrack::insertGroup(Array<TimeSignatureEvent>& signatures, bool undoable)
{
    if (undoable)
    {
        project.getUndoManager().
            perform(new TimeSignatureEventsGroupInsertAction(project,
                getTrackId(), signatures));
    }
    else
    {
        for (int i = 0; i < signatures.size(); ++i)
        {
            const TimeSignatureEvent& eventParams = signatures.getReference(i);
            auto* ownedEvent = new TimeSignatureEvent(this, eventParams);
            midiEvents.addSorted(*ownedEvent, ownedEvent);
            project.broadcastAddEvent(*ownedEvent);
        }
        updateBeatRange(true);
        project.broadcastPostAddEvent();
    }

    return true;
}

bool TimeSignatureTrack::removeGroup(Array<TimeSignatureEvent>& signatures, bool undoable)
{
    if (undoable)
    {
        project.getUndoManager().
            perform(new TimeSignatureEventsGroupRemoveAction(project,
                getTrackId(), signatures));
    }
    else
    {
        for (int i = 0; i < signatures.size(); ++i)
        {
            const TimeSignatureEvent& signature = signatures.getReference(i);
            const int index = midiEvents.indexOfSorted(signature, &signature);
            if (index >= 0)
            {
                auto* removedSignature = midiEvents.getUnchecked(index);
                project.broadcastRemoveEvent(*removedSignature);
                midiEvents.remove(index, true);
            }
        }
        updateBeatRange(true);
        project.broadcastPostRemoveEvent(this);
    }

    return true;
}

bool TimeSignatureTrack::changeGroup(Array<TimeSignatureEvent>& groupBefore,
    Array<TimeSignatureEvent>& groupAfter, bool undoable)
{
    jassert(groupBefore.size() == groupAfter.size());

    if (undoable)
    {
        project.getUndoManager().
            perform(new TimeSignatureEventsGroupChangeAction(project,
                getTrackId(), groupBefore, groupAfter));
    }
    else
    {
        for (int i = 0; i < groupBefore.size(); ++i)
        {
            const TimeSignatureEvent& oldParams = groupBefore.getReference(i);
            const TimeSignatureEvent& newParams = groupAfter.getReference(i);
            const int index = midiEvents.indexOfSorted(oldParams, &oldParams);
            if (index >= 0)
            {
                auto* changedEvent = static_cast<TimeSignatureEvent*>(midiEvents.getUnchecked(index));
                changedEvent->applyChanges(newParams);
                midiEvents.remove(index, false);
                midiEvents.addSorted(*changedEvent, changedEvent);
                project.broadcastChangeEvent(oldParams, *changedEvent);
            }
        }
        updateBeatRange(true);
        project.broadcastPostChangeEvent();
    }

    return true;
}

//===----------------------------------------------------------------------===//
// Serializable
//===----------------------------------------------------------------------===//

ValueTree TimeSignatureTrack::serialize() const
{
    ValueTree tree(Serialization::Midi::timeSignatures);

    for (int i = 0; i < midiEvents.size(); ++i)
    {
        const MidiEvent* event = midiEvents.getUnchecked(i);
        tree.appendChild(event->serialize(), nullptr); // faster than addChildElement
    }

    return tree;
}

void TimeSignatureTrack::deserialize(const ValueTree& tree)
{
    reset();
    using namespace Serialization;

    const auto root =
        tree.hasType(Serialization::Midi::timeSignatures) ?
        tree : tree.getChildWithName(Serialization::Midi::timeSignatures);

    if (!root.isValid())
        return;

    float lastBeat = 0;
    float firstBeat = 0;

    for (const auto& e : root)
    {
        if (e.hasType(Serialization::Midi::timeSignature))
        {
            TimeSignatureEvent* signature = new TimeSignatureEvent(this);
            signature->deserialize(e);

            midiEvents.add(signature);

            lastBeat = jmax(lastBeat, signature->getBeat());
            firstBeat = jmin(firstBeat, signature->getBeat());
        }
    }

    sort();
    updateBeatRange(false);
}

void TimeSignatureTrack::reset()
{
    midiEvents.clear();
}
