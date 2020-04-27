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

#include "MidiTrack.h"

#include "Project.h"
#include "MidiTrackActions.h"

//==============================================================================
MidiTrack::MidiTrack(Project& owner) noexcept :
    project(owner),
    name("New Track"),
    channel(1),
    lastStartBeat(0.f),
    lastEndBeat(0.f)
{
    id = IdGenerator::generateId();
}

//===----------------------------------------------------------------------===//
// Accessors
//===----------------------------------------------------------------------===//
IdGenerator::Id MidiTrack::getTrackId() const noexcept
{
    return id;
}

float MidiTrack::getFirstBeat() const noexcept
{
    if (this->midiEvents.size() == 0)
    {
        return FLT_MAX;
    }

    return midiEvents.getFirst()->getBeat();
}

float MidiTrack::getLastBeat() const noexcept
{
    if (this->midiEvents.size() == 0)
    {
        return -FLT_MAX;
    }

    return midiEvents.getLast()->getBeat();
}

int MidiTrack::getTrackChannel() const noexcept
{
    return channel;
}

void MidiTrack::setTrackChannel(int val, bool sendNotifications, bool undoable)
{
    if (channel == val)
        return;
    if (undoable)
    {
        project.getUndoManager().
            perform(new MidiTrackChannelChangeAction(project,
                getTrackId(), val));
    }
    else
    {
        channel = val;

        if (sendNotification)
            project.broadcastChangeTrackProperties(this);
    }
}

String MidiTrack::getTrackName() const noexcept
{
    return name;
}

void MidiTrack::setTrackName(const String& val, bool sendNotifications, bool undoable)
{
    if (name == val)
        return;
    if (undoable)
    {
        project.getUndoManager().
            perform(new MidiTrackRenameAction(project,
                getTrackId(), val));
    }
    else
    {
        name = val;

        if (sendNotification)
            project.broadcastChangeTrackProperties(this);
    }
}

//===------------------------------------------------------------------===//
// OwnedArray wrapper
//===------------------------------------------------------------------===//
void MidiTrack::sort()
{
    if (this->midiEvents.size() > 0)
    {
        this->midiEvents.sort(*this->midiEvents.getFirst(), true);
    }
}

//===------------------------------------------------------------------===//
// Helpers
//===------------------------------------------------------------------===//
void MidiTrack::updateBeatRange(bool shouldNotifyIfChanged)
{
    lastStartBeat = getFirstBeat();
    lastEndBeat = getLastBeat();

    if (shouldNotifyIfChanged)
    {
        project.broadcastChangeProjectBeatRange();
    }
}

void MidiTrack::reset()
{
    midiEvents.clear();
}

int MidiTrack::compareElements(const MidiTrack* const first, const MidiTrack* const second) noexcept
{
    if (first == second)
        return 0;

    const float nameResult = first->name.compare(second->name);
    if (nameResult != 0) return nameResult;

    const float channelDiff = first->channel - second->channel;
    const int channelResult = (channelDiff > 0.f) - (channelDiff < 0.f);
    if (channelResult != 0) return channelResult;

    const float idDiff = first->id - second->id;
    return (idDiff > 0.f) - (idDiff < 0.f);
}
