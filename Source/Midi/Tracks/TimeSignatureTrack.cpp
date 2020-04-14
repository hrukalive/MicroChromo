/*
  ==============================================================================

    TimeSignatureTrack.cpp
    Created: 13 Apr 2020 11:46:41am
    Author:  bowen

  ==============================================================================
*/

#include "TimeSignatureTrack.h"
#include "Project.h"

TimeSignaturesTrack::TimeSignaturesTrack(Project& project) noexcept :
    MidiTrack(project) {}

//===----------------------------------------------------------------------===//
// Import/export
//===----------------------------------------------------------------------===//

void TimeSignaturesTrack::importMidi(const MidiMessageSequence& sequence, short timeFormat)
{
    this->reset();
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
            this->importMidiEvent<TimeSignatureEvent>(signature);
        }
    }

    this->updateBeatRange(false);
}

//===----------------------------------------------------------------------===//
// Undoable track editing
//===----------------------------------------------------------------------===//

MidiEvent* TimeSignaturesTrack::insert(const TimeSignatureEvent& eventParams, bool undoable)
{
    if (undoable)
    {
        this->getUndoStack()->
            perform(new TimeSignatureEventInsertAction(*this->getProject(),
                this->getTrackId(), eventParams));
    }
    else
    {
        auto* ownedEvent = new TimeSignatureEvent(this, eventParams);
        midiEvents.addSorted(*ownedEvent, ownedEvent);
        project.broadcastAddEvent(*ownedEvent);
        updateBeatRange(true);
        return ownedEvent;
    }

    return nullptr;
}

bool TimeSignaturesTrack::remove(const TimeSignatureEvent& signature, bool undoable)
{
    if (undoable)
    {
        this->getUndoStack()->
            perform(new TimeSignatureEventRemoveAction(*this->getProject(),
                this->getTrackId(), signature));
    }
    else
    {
        const int index = this->midiEvents.indexOfSorted(signature, &signature);
        if (index >= 0)
        {
            auto* removedEvent = this->midiEvents.getUnchecked(index);
            project.broadcastRemoveEvent(*removedEvent);
            this->midiEvents.remove(index, true);
            this->updateBeatRange(true);
            project.broadcastPostRemoveEvent(this);
            return true;
        }

        return false;
    }

    return true;
}

bool TimeSignaturesTrack::change(const TimeSignatureEvent& oldParams,
    const TimeSignatureEvent& newParams, bool undoable)
{
    if (undoable)
    {
        this->getUndoStack()->
            perform(new TimeSignatureEventChangeAction(*this->getProject(),
                this->getTrackId(), oldParams, newParams));
    }
    else
    {
        const int index = this->midiEvents.indexOfSorted(oldParams, &oldParams);
        if (index >= 0)
        {
            auto* changedEvent = static_cast<TimeSignatureEvent*>(this->midiEvents.getUnchecked(index));
            changedEvent->applyChanges(newParams);
            this->midiEvents.remove(index, false);
            this->midiEvents.addSorted(*changedEvent, changedEvent);
            project.broadcastChangeEvent(oldParams, *changedEvent);
            this->updateBeatRange(true);
            return true;
        }

        return false;
    }

    return true;
}

bool TimeSignaturesTrack::insertGroup(Array<TimeSignatureEvent>& signatures, bool undoable)
{
    if (undoable)
    {
        this->getUndoStack()->
            perform(new TimeSignatureEventsGroupInsertAction(*this->getProject(),
                this->getTrackId(), signatures));
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

        this->updateBeatRange(true);
    }

    return true;
}

bool TimeSignaturesTrack::removeGroup(Array<TimeSignatureEvent>& signatures, bool undoable)
{
    if (undoable)
    {
        this->getUndoStack()->
            perform(new TimeSignatureEventsGroupRemoveAction(*this->getProject(),
                this->getTrackId(), signatures));
    }
    else
    {
        for (int i = 0; i < signatures.size(); ++i)
        {
            const TimeSignatureEvent& signature = signatures.getReference(i);
            const int index = this->midiEvents.indexOfSorted(signature, &signature);
            if (index >= 0)
            {
                auto* removedSignature = this->midiEvents.getUnchecked(index);
                project.broadcastRemoveEvent(*removedSignature);
                this->midiEvents.remove(index, true);
            }
        }

        this->updateBeatRange(true);
        project.broadcastPostRemoveEvent(this);
    }

    return true;
}

bool TimeSignaturesTrack::changeGroup(Array<TimeSignatureEvent>& groupBefore,
    Array<TimeSignatureEvent>& groupAfter, bool undoable)
{
    jassert(groupBefore.size() == groupAfter.size());

    if (undoable)
    {
        this->getUndoStack()->
            perform(new TimeSignatureEventsGroupChangeAction(*this->getProject(),
                this->getTrackId(), groupBefore, groupAfter));
    }
    else
    {
        for (int i = 0; i < groupBefore.size(); ++i)
        {
            const TimeSignatureEvent& oldParams = groupBefore.getReference(i);
            const TimeSignatureEvent& newParams = groupAfter.getReference(i);
            const int index = this->midiEvents.indexOfSorted(oldParams, &oldParams);
            if (index >= 0)
            {
                auto* changedEvent = static_cast<TimeSignatureEvent*>(this->midiEvents.getUnchecked(index));
                changedEvent->applyChanges(newParams);
                this->midiEvents.remove(index, false);
                this->midiEvents.addSorted(*changedEvent, changedEvent);
                project.broadcastChangeEvent(oldParams, *changedEvent);
            }
        }

        this->updateBeatRange(true);
    }

    return true;
}

//===----------------------------------------------------------------------===//
// Serializable
//===----------------------------------------------------------------------===//

ValueTree TimeSignaturesTrack::serialize() const
{
    ValueTree tree(Serialization::Midi::timeSignatures);

    for (int i = 0; i < midiEvents.size(); ++i)
    {
        const MidiEvent* event = this->midiEvents.getUnchecked(i);
        tree.appendChild(event->serialize(), nullptr); // faster than addChildElement
    }

    return tree;
}

void TimeSignaturesTrack::deserialize(const ValueTree& tree)
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

            this->midiEvents.add(signature);

            lastBeat = jmax(lastBeat, signature->getBeat());
            firstBeat = jmin(firstBeat, signature->getBeat());
        }
    }

    this->sort();
    this->updateBeatRange(false);
}

void TimeSignaturesTrack::reset()
{
    this->midiEvents.clear();
}
