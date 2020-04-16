/*
  ==============================================================================

    TempoTrack.cpp
    Created: 13 Apr 2020 11:46:46am
    Author:  bowen

  ==============================================================================
*/

#include "TempoTrack.h"
#include "TempoMarkerEventActions.h"
#include "Project.h"

TempoTrack::TempoTrack(Project& project) noexcept :
    MidiTrack(project) {}

//===----------------------------------------------------------------------===//
// Import/export
//===----------------------------------------------------------------------===//

void TempoTrack::importMidi(const MidiMessageSequence& sequence, short timeFormat)
{
    reset();
    for (int i = 0; i < sequence.getNumEvents(); ++i)
    {
        const MidiMessage& message = sequence.getEventPointer(i)->message;
        if (message.isTempoMetaEvent())
        {
            int numerator = 0;
            int denominator = 0;
            auto bpm = int(60.0f / message.getTempoSecondsPerQuarterNote());
            const float startBeat = MidiTrack::midiTicksToBeats(message.getTimeStamp(), timeFormat);
            const TempoMarkerEvent marker(this, startBeat, bpm);
            importMidiEvent<TempoMarkerEvent>(marker);
        }
    }

    updateBeatRange(false);
}

//===----------------------------------------------------------------------===//
// Undoable track editing
//===----------------------------------------------------------------------===//

MidiEvent* TempoTrack::insert(const TempoMarkerEvent& eventParams, bool undoable)
{
    if (undoable)
    {
        project.getUndoManager().
            perform(new TempoMarkerEventInsertAction(project,
                getTrackId(), eventParams));
    }
    else
    {
        auto* ownedEvent = new TempoMarkerEvent(this, eventParams);
        midiEvents.addSorted(*ownedEvent, ownedEvent);
        project.broadcastAddEvent(*ownedEvent);
        updateBeatRange(true);
        project.broadcastPostAddEvent();
        return ownedEvent;
    }

    return nullptr;
}

bool TempoTrack::remove(const TempoMarkerEvent& signature, bool undoable)
{
    if (undoable)
    {
        project.getUndoManager().
            perform(new TempoMarkerEventRemoveAction(project,
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

bool TempoTrack::change(const TempoMarkerEvent& oldParams,
    const TempoMarkerEvent& newParams, bool undoable)
{
    if (undoable)
    {
        project.getUndoManager().
            perform(new TempoMarkerEventChangeAction(project,
                getTrackId(), oldParams, newParams));
    }
    else
    {
        const int index = midiEvents.indexOfSorted(oldParams, &oldParams);
        if (index >= 0)
        {
            auto* changedEvent = static_cast<TempoMarkerEvent*>(midiEvents.getUnchecked(index));
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

bool TempoTrack::insertGroup(Array<TempoMarkerEvent>& signatures, bool undoable)
{
    if (undoable)
    {
        project.getUndoManager().
            perform(new TempoMarkerEventsGroupInsertAction(project,
                getTrackId(), signatures));
    }
    else
    {
        for (int i = 0; i < signatures.size(); ++i)
        {
            const TempoMarkerEvent& eventParams = signatures.getReference(i);
            auto* ownedEvent = new TempoMarkerEvent(this, eventParams);
            midiEvents.addSorted(*ownedEvent, ownedEvent);
            project.broadcastAddEvent(*ownedEvent);
        }
        updateBeatRange(true);
        project.broadcastPostAddEvent();
    }

    return true;
}

bool TempoTrack::removeGroup(Array<TempoMarkerEvent>& signatures, bool undoable)
{
    if (undoable)
    {
        project.getUndoManager().
            perform(new TempoMarkerEventsGroupRemoveAction(project,
                getTrackId(), signatures));
    }
    else
    {
        for (int i = 0; i < signatures.size(); ++i)
        {
            const TempoMarkerEvent& signature = signatures.getReference(i);
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

bool TempoTrack::changeGroup(Array<TempoMarkerEvent>& groupBefore,
    Array<TempoMarkerEvent>& groupAfter, bool undoable)
{
    jassert(groupBefore.size() == groupAfter.size());

    if (undoable)
    {
        project.getUndoManager().
            perform(new TempoMarkerEventsGroupChangeAction(project,
                getTrackId(), groupBefore, groupAfter));
    }
    else
    {
        for (int i = 0; i < groupBefore.size(); ++i)
        {
            const TempoMarkerEvent& oldParams = groupBefore.getReference(i);
            const TempoMarkerEvent& newParams = groupAfter.getReference(i);
            const int index = midiEvents.indexOfSorted(oldParams, &oldParams);
            if (index >= 0)
            {
                auto* changedEvent = static_cast<TempoMarkerEvent*>(midiEvents.getUnchecked(index));
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

ValueTree TempoTrack::serialize() const
{
    ValueTree tree(Serialization::Midi::tempoMarker);

    for (int i = 0; i < midiEvents.size(); ++i)
    {
        const MidiEvent* event = midiEvents.getUnchecked(i);
        tree.appendChild(event->serialize(), nullptr); // faster than addChildElement
    }

    return tree;
}

void TempoTrack::deserialize(const ValueTree& tree)
{
    reset();
    using namespace Serialization;

    const auto root =
        tree.hasType(Serialization::Midi::tempoMarker) ?
        tree : tree.getChildWithName(Serialization::Midi::tempoMarker);

    if (!root.isValid())
        return;

    float lastBeat = 0;
    float firstBeat = 0;

    for (const auto& e : root)
    {
        if (e.hasType(Serialization::Midi::tempoMarker))
        {
            TempoMarkerEvent* marker = new TempoMarkerEvent(this);
            marker->deserialize(e);

            midiEvents.add(marker);

            lastBeat = jmax(lastBeat, marker->getBeat());
            firstBeat = jmin(firstBeat, marker->getBeat());
        }
    }

    sort();
    updateBeatRange(false);
}

void TempoTrack::reset()
{
    midiEvents.clear();
}
