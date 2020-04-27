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

#include "MidiTrackActions.h"

#include "SerializationKeys.h"
#include "Project.h"
#include "NoteTrack.h"

//===----------------------------------------------------------------------===//
// Rename
//===----------------------------------------------------------------------===//
MidiTrackRenameAction::MidiTrackRenameAction(Project& project, 
    IdGenerator::Id trackId, const String& name) noexcept :
    project(project),
    trackId(trackId),
    nameAfter(name) {}

bool MidiTrackRenameAction::perform()
{
    if (MidiTrack* track = project.findTrackById<MidiTrack>(trackId))
    {
        nameBefore = track->getTrackName();
        track->setTrackName(nameAfter, true, false);
        return true;
    }

    return false;
}

bool MidiTrackRenameAction::undo()
{
    if (MidiTrack* track = project.findTrackById<MidiTrack>(trackId))
    {
        track->setTrackName(nameBefore, true, false);
        return true;
    }

    return false;
}

int MidiTrackRenameAction::getSizeInUnits()
{
    return nameBefore.length() + nameAfter.length();
}


//===----------------------------------------------------------------------===//
// Channel
//===----------------------------------------------------------------------===//
MidiTrackChannelChangeAction::MidiTrackChannelChangeAction(Project& project,
    IdGenerator::Id trackId, int channel) noexcept :
    project(project),
    trackId(trackId),
    channelAfter(channel) {}

bool MidiTrackChannelChangeAction::perform()
{
    if (MidiTrack* track = project.findTrackById<MidiTrack>(trackId))
    {
        channelBefore = track->getTrackChannel();
        track->setTrackChannel(channelAfter, true, false);
        return true;
    }

    return false;
}

bool MidiTrackChannelChangeAction::undo()
{
    if (MidiTrack* track = project.findTrackById<MidiTrack>(trackId))
    {
        track->setTrackChannel(channelBefore, true, false);
        return true;
    }

    return false;
}

int MidiTrackChannelChangeAction::getSizeInUnits()
{
    return 2 * sizeof(int);
}
