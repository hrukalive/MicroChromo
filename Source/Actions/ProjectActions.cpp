/*
  ==============================================================================

    ProjectActions.cpp
    Created: 14 Apr 2020 7:05:52pm
    Author:  bowen

  ==============================================================================
*/

#include "ProjectActions.h"
#include "Project.h"
#include "NoteTrack.h"

// Note track ID is reference in note actions, therefore, not changing.

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
