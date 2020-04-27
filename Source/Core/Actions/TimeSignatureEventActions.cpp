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

#include "TimeSignatureEventActions.h"

#include "SerializationKeys.h"
#include "Project.h"
#include "TimeSignatureTrack.h"

//===----------------------------------------------------------------------===//
// Insert
//===----------------------------------------------------------------------===//
TimeSignatureEventInsertAction::TimeSignatureEventInsertAction(Project& project,
    IdGenerator::Id trackId, const TimeSignatureEvent& event) noexcept :
    project(project),
    trackId(trackId),
    event(event) {}

bool TimeSignatureEventInsertAction::perform()
{
    if (TimeSignatureTrack* track = project.findTrackById<TimeSignatureTrack>(trackId))
    {
        return (track->insert(event, false) != nullptr);
    }

    return false;
}

bool TimeSignatureEventInsertAction::undo()
{
    if (TimeSignatureTrack* track = project.findTrackById<TimeSignatureTrack>(trackId))
    {
        return track->remove(event, false);
    }

    return false;
}

int TimeSignatureEventInsertAction::getSizeInUnits()
{
    return sizeof(TimeSignatureEvent);
}

//===----------------------------------------------------------------------===//
// Remove
//===----------------------------------------------------------------------===//
TimeSignatureEventRemoveAction::TimeSignatureEventRemoveAction(Project& project,
    IdGenerator::Id trackId, const TimeSignatureEvent& target) noexcept :
    project(project),
    trackId(trackId),
    event(target) {}

bool TimeSignatureEventRemoveAction::perform()
{
    if (TimeSignatureTrack* track = project.findTrackById<TimeSignatureTrack>(trackId))
    {
        return track->remove(event, false);
    }

    return false;
}

bool TimeSignatureEventRemoveAction::undo()
{
    if (TimeSignatureTrack* track = project.findTrackById<TimeSignatureTrack>(trackId))
    {
        return (track->insert(event, false) != nullptr);
    }

    return false;
}

int TimeSignatureEventRemoveAction::getSizeInUnits()
{
    return sizeof(TimeSignatureEvent);
}

//===----------------------------------------------------------------------===//
// Change
//===----------------------------------------------------------------------===//
TimeSignatureEventChangeAction::TimeSignatureEventChangeAction(Project& project,
    IdGenerator::Id trackId, const TimeSignatureEvent& target, const TimeSignatureEvent& newParameters) noexcept :
    project(project),
    trackId(trackId),
    eventBefore(target),
    eventAfter(newParameters)
{
    jassert(target.getId() == newParameters.getId());
}

bool TimeSignatureEventChangeAction::perform()
{
    if (TimeSignatureTrack* track = project.findTrackById<TimeSignatureTrack>(trackId))
    {
        return track->change(eventBefore, eventAfter, false);
    }

    return false;
}

bool TimeSignatureEventChangeAction::undo()
{
    if (TimeSignatureTrack* track = project.findTrackById<TimeSignatureTrack>(trackId))
    {
        return track->change(eventAfter, eventBefore, false);
    }

    return false;
}

int TimeSignatureEventChangeAction::getSizeInUnits()
{
    return sizeof(TimeSignatureEvent) * 2;
}

UndoableAction* TimeSignatureEventChangeAction::createCoalescedAction(UndoableAction* nextAction)
{
    if (TimeSignatureTrack* track = project.findTrackById<TimeSignatureTrack>(trackId))
    {
        if (TimeSignatureEventChangeAction* nextChanger =
            dynamic_cast<TimeSignatureEventChangeAction*>(nextAction))
        {
            const bool idsAreEqual =
                (eventBefore.getId() == nextChanger->eventAfter.getId() &&
                    trackId == nextChanger->trackId);

            if (idsAreEqual)
            {
                return new TimeSignatureEventChangeAction(project,
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
TimeSignatureEventsGroupInsertAction::TimeSignatureEventsGroupInsertAction(Project& project,
    IdGenerator::Id trackId, Array<TimeSignatureEvent>& target) noexcept :
    project(project),
    trackId(trackId)
{
    signatures.swapWith(target);
}

bool TimeSignatureEventsGroupInsertAction::perform()
{
    if (TimeSignatureTrack* track = project.findTrackById<TimeSignatureTrack>(trackId))
    {
        return track->insertGroup(signatures, false);
    }

    return false;
}

bool TimeSignatureEventsGroupInsertAction::undo()
{
    if (TimeSignatureTrack* track = project.findTrackById<TimeSignatureTrack>(trackId))
    {
        return track->removeGroup(signatures, false);
    }

    return false;
}

int TimeSignatureEventsGroupInsertAction::getSizeInUnits()
{
    return (sizeof(TimeSignatureEvent) * signatures.size());
}

//===----------------------------------------------------------------------===//
// Remove Group
//===----------------------------------------------------------------------===//
TimeSignatureEventsGroupRemoveAction::TimeSignatureEventsGroupRemoveAction(Project& project,
    IdGenerator::Id trackId, Array<TimeSignatureEvent>& target) noexcept :
    project(project),
    trackId(trackId)
{
    signatures.swapWith(target);
}

bool TimeSignatureEventsGroupRemoveAction::perform()
{
    if (TimeSignatureTrack* track = project.findTrackById<TimeSignatureTrack>(trackId))
    {
        return track->removeGroup(signatures, false);
    }

    return false;
}

bool TimeSignatureEventsGroupRemoveAction::undo()
{
    if (TimeSignatureTrack* track = project.findTrackById<TimeSignatureTrack>(trackId))
    {
        return track->insertGroup(signatures, false);
    }

    return false;
}

int TimeSignatureEventsGroupRemoveAction::getSizeInUnits()
{
    return (sizeof(TimeSignatureEvent) * signatures.size());
}

//===----------------------------------------------------------------------===//
// Change Group
//===----------------------------------------------------------------------===//
TimeSignatureEventsGroupChangeAction::TimeSignatureEventsGroupChangeAction(Project& project,
    IdGenerator::Id trackId, Array<TimeSignatureEvent>& state1, Array<TimeSignatureEvent>& state2) noexcept :
    project(project),
    trackId(trackId)
{
    eventsBefore.swapWith(state1);
    eventsAfter.swapWith(state2);
}

bool TimeSignatureEventsGroupChangeAction::perform()
{
    if (TimeSignatureTrack* track = project.findTrackById<TimeSignatureTrack>(trackId))
    {
        return track->changeGroup(eventsBefore, eventsAfter, false);
    }

    return false;
}

bool TimeSignatureEventsGroupChangeAction::undo()
{
    if (TimeSignatureTrack* track = project.findTrackById<TimeSignatureTrack>(trackId))
    {
        return track->changeGroup(eventsAfter, eventsBefore, false);
    }

    return false;
}

int TimeSignatureEventsGroupChangeAction::getSizeInUnits()
{
    return (sizeof(TimeSignatureEvent) * eventsBefore.size()) +
        (sizeof(TimeSignatureEvent) * eventsAfter.size());
}

UndoableAction* TimeSignatureEventsGroupChangeAction::createCoalescedAction(UndoableAction* nextAction)
{
    if (TimeSignatureTrack* track = project.findTrackById<TimeSignatureTrack>(trackId))
    {
        if (TimeSignatureEventsGroupChangeAction* nextChanger =
            dynamic_cast<TimeSignatureEventsGroupChangeAction*>(nextAction))
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

            return new TimeSignatureEventsGroupChangeAction(project,
                trackId, eventsBefore, nextChanger->eventsAfter);
        }
    }

    (void)nextAction;
    return nullptr;
}
