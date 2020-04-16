/*
  ==============================================================================

    NoteTrackActions.cpp
    Created: 14 Apr 2020 6:28:06pm
    Author:  bowen

  ==============================================================================
*/

#include "NoteTrackActions.h"
#include "SerializationKeys.h"
#include "NoteTrack.h"
#include "Project.h"

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
