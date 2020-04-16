
#include "Common.h"
#include "SerializationKeys.h"
#include "TempoMarkerEventActions.h"
#include "MidiTrack.h"
#include "TempoTrack.h"
#include "Project.h"

//===----------------------------------------------------------------------===//
// Insert
//===----------------------------------------------------------------------===//

TempoMarkerEventInsertAction::TempoMarkerEventInsertAction(Project& project,
    IdGenerator::Id trackId, const TempoMarkerEvent& event) noexcept :
    project(project),
    trackId(trackId),
    event(event) {}

bool TempoMarkerEventInsertAction::perform()
{
    if (TempoTrack* track = project.findTrackById<TempoTrack>(trackId))
    {
        return (track->insert(event, false) != nullptr);
    }

    return false;
}

bool TempoMarkerEventInsertAction::undo()
{
    if (TempoTrack* track = project.findTrackById<TempoTrack>(trackId))
    {
        return track->remove(event, false);
    }

    return false;
}

int TempoMarkerEventInsertAction::getSizeInUnits()
{
    return sizeof(TempoMarkerEvent);
}

//===----------------------------------------------------------------------===//
// Remove
//===----------------------------------------------------------------------===//

TempoMarkerEventRemoveAction::TempoMarkerEventRemoveAction(Project& project,
    IdGenerator::Id trackId, const TempoMarkerEvent& target) noexcept :
    project(project),
    trackId(trackId),
    event(target) {}

bool TempoMarkerEventRemoveAction::perform()
{
    if (TempoTrack* track = project.findTrackById<TempoTrack>(trackId))
    {
        return track->remove(event, false);
    }

    return false;
}

bool TempoMarkerEventRemoveAction::undo()
{
    if (TempoTrack* track = project.findTrackById<TempoTrack>(trackId))
    {
        return (track->insert(event, false) != nullptr);
    }

    return false;
}

int TempoMarkerEventRemoveAction::getSizeInUnits()
{
    return sizeof(TempoMarkerEvent);
}

//===----------------------------------------------------------------------===//
// Change
//===----------------------------------------------------------------------===//

TempoMarkerEventChangeAction::TempoMarkerEventChangeAction(Project& project,
    IdGenerator::Id trackId, const TempoMarkerEvent& target, const TempoMarkerEvent& newParameters) noexcept :
    project(project),
    trackId(trackId),
    eventBefore(target),
    eventAfter(newParameters)
{
    jassert(target.getId() == newParameters.getId());
}

bool TempoMarkerEventChangeAction::perform()
{
    if (TempoTrack* track = project.findTrackById<TempoTrack>(trackId))
    {
        return track->change(eventBefore, eventAfter, false);
    }

    return false;
}

bool TempoMarkerEventChangeAction::undo()
{
    if (TempoTrack* track = project.findTrackById<TempoTrack>(trackId))
    {
        return track->change(eventAfter, eventBefore, false);
    }

    return false;
}

int TempoMarkerEventChangeAction::getSizeInUnits()
{
    return sizeof(TempoMarkerEvent) * 2;
}

UndoableAction* TempoMarkerEventChangeAction::createCoalescedAction(UndoableAction* nextAction)
{
    if (TempoTrack* track = project.findTrackById<TempoTrack>(trackId))
    {
        if (TempoMarkerEventChangeAction* nextChanger =
            dynamic_cast<TempoMarkerEventChangeAction*>(nextAction))
        {
            const bool idsAreEqual =
                (eventBefore.getId() == nextChanger->eventAfter.getId() &&
                    trackId == nextChanger->trackId);

            if (idsAreEqual)
            {
                return new TempoMarkerEventChangeAction(project,
                    trackId, eventBefore, nextChanger->eventAfter);
            }
        }
    }

    (void)nextAction;
    return nullptr;
}

//===----------------------------------------------------------------------===//
// Insert Group
//===----------------------------------------------------------------------===//

TempoMarkerEventsGroupInsertAction::TempoMarkerEventsGroupInsertAction(Project& project,
    IdGenerator::Id trackId, Array<TempoMarkerEvent>& target) noexcept :
    project(project),
    trackId(trackId)
{
    signatures.swapWith(target);
}

bool TempoMarkerEventsGroupInsertAction::perform()
{
    if (TempoTrack* track = project.findTrackById<TempoTrack>(trackId))
    {
        return track->insertGroup(signatures, false);
    }

    return false;
}

bool TempoMarkerEventsGroupInsertAction::undo()
{
    if (TempoTrack* track = project.findTrackById<TempoTrack>(trackId))
    {
        return track->removeGroup(signatures, false);
    }

    return false;
}

int TempoMarkerEventsGroupInsertAction::getSizeInUnits()
{
    return (sizeof(TempoMarkerEvent) * signatures.size());
}

//===----------------------------------------------------------------------===//
// Remove Group
//===----------------------------------------------------------------------===//

TempoMarkerEventsGroupRemoveAction::TempoMarkerEventsGroupRemoveAction(Project& project,
    IdGenerator::Id trackId, Array<TempoMarkerEvent>& target) noexcept :
    project(project),
    trackId(trackId)
{
    signatures.swapWith(target);
}

bool TempoMarkerEventsGroupRemoveAction::perform()
{
    if (TempoTrack* track = project.findTrackById<TempoTrack>(trackId))
    {
        return track->removeGroup(signatures, false);
    }

    return false;
}

bool TempoMarkerEventsGroupRemoveAction::undo()
{
    if (TempoTrack* track = project.findTrackById<TempoTrack>(trackId))
    {
        return track->insertGroup(signatures, false);
    }

    return false;
}

int TempoMarkerEventsGroupRemoveAction::getSizeInUnits()
{
    return (sizeof(TempoMarkerEvent) * signatures.size());
}

//===----------------------------------------------------------------------===//
// Change Group
//===----------------------------------------------------------------------===//

TempoMarkerEventsGroupChangeAction::TempoMarkerEventsGroupChangeAction(Project& project,
    IdGenerator::Id trackId, Array<TempoMarkerEvent>& state1, Array<TempoMarkerEvent>& state2) noexcept :
    project(project),
    trackId(trackId)
{
    eventsBefore.swapWith(state1);
    eventsAfter.swapWith(state2);
}

bool TempoMarkerEventsGroupChangeAction::perform()
{
    if (TempoTrack* track = project.findTrackById<TempoTrack>(trackId))
    {
        return track->changeGroup(eventsBefore, eventsAfter, false);
    }

    return false;
}

bool TempoMarkerEventsGroupChangeAction::undo()
{
    if (TempoTrack* track = project.findTrackById<TempoTrack>(trackId))
    {
        return track->changeGroup(eventsAfter, eventsBefore, false);
    }

    return false;
}

int TempoMarkerEventsGroupChangeAction::getSizeInUnits()
{
    return (sizeof(TempoMarkerEvent) * eventsBefore.size()) +
        (sizeof(TempoMarkerEvent) * eventsAfter.size());
}

UndoableAction* TempoMarkerEventsGroupChangeAction::createCoalescedAction(UndoableAction* nextAction)
{
    if (TempoTrack* track = project.findTrackById<TempoTrack>(trackId))
    {
        if (TempoMarkerEventsGroupChangeAction* nextChanger =
            dynamic_cast<TempoMarkerEventsGroupChangeAction*>(nextAction))
        {
            if (nextChanger->trackId != trackId)
            {
                return nullptr;
            }

            if (eventsBefore.size() != nextChanger->eventsAfter.size())
            {
                return nullptr;
            }

            for (int i = 0; i < eventsBefore.size(); ++i)
            {
                if (eventsBefore.getUnchecked(i).getId() !=
                    nextChanger->eventsAfter.getUnchecked(i).getId())
                {
                    return nullptr;
                }
            }

            return new TempoMarkerEventsGroupChangeAction(project,
                trackId, eventsBefore, nextChanger->eventsAfter);
        }
    }

    (void)nextAction;
    return nullptr;
}
