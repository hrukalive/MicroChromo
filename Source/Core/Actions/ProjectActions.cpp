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

#include "ProjectActions.h"

#include "Project.h"
#include "NoteTrack.h"

//===----------------------------------------------------------------------===//
// Insert
//===----------------------------------------------------------------------===//
NoteTrackInsertAction::NoteTrackInsertAction(Project& project,
    ValueTree& targetSerializedState,
    const String& name, int channel) noexcept :
    project(project),
    trackState(targetSerializedState),
    trackName(name),
    trackChannel(channel) {}

bool NoteTrackInsertAction::perform()
{
    auto* track = project.addTrack(trackState, trackName, trackChannel, false);
    trackId = track->getTrackId();
    trackState = track->serializeWithId();
    DBG("Perform [Insert track] " << trackId);
    return track != nullptr;
}

bool NoteTrackInsertAction::undo()
{
    DBG("Undo [Insert track] " << trackId);
    return project.removeTrack(trackId, false);
}

int NoteTrackInsertAction::getSizeInUnits()
{
    return trackName.length();
}


//===----------------------------------------------------------------------===//
// Remove
//===----------------------------------------------------------------------===//
NoteTrackRemoveAction::NoteTrackRemoveAction(Project& project, 
    IdGenerator::Id trackId) noexcept :
    project(project),
    trackId(trackId),
    numEvents(0) {}

bool NoteTrackRemoveAction::perform()
{
    if (NoteTrack* track = project.findTrackById<NoteTrack>(trackId))
    {
        numEvents = track->size();
        serializedTreeItem = track->serializeWithId();
        DBG("Perform [Remove track] " << trackId);
        return project.removeTrack(trackId, false);
    }
    return false;
}

bool NoteTrackRemoveAction::undo()
{
    if (serializedTreeItem.isValid())
    {
        auto* track = project.addTrack(serializedTreeItem, trackName, trackChannel, false);
        DBG("Undo [Remove track] " << track->getTrackId());
        return track != nullptr;
    }

    return false;
}

int NoteTrackRemoveAction::getSizeInUnits()
{
    if (serializedTreeItem.isValid())
        return (numEvents * sizeof(MidiEvent));
    return 1;
}
