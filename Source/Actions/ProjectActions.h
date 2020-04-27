/*
  ==============================================================================

    ProjectActions.h
    Created: 14 Apr 2020 7:05:52pm
    Author:  bowen

  ==============================================================================
*/

#pragma once

#include "Common.h"
#include "NoteTrack.h"

class Project;

//===----------------------------------------------------------------------===//
// Insert
//===----------------------------------------------------------------------===//

class NoteTrackInsertAction final : public UndoableAction
{
public:
    explicit NoteTrackInsertAction(Project& project) noexcept : project(project) {}
    NoteTrackInsertAction(Project& project, ValueTree& serializedState, 
        const String& name, int channel) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;

    IdGenerator::Id getTrackId() const noexcept { return trackId; }

private:
    Project& project;

    IdGenerator::Id trackId = -1;
    String trackName;
    int trackChannel;

    ValueTree trackState;

    JUCE_DECLARE_NON_COPYABLE(NoteTrackInsertAction)
};

//===----------------------------------------------------------------------===//
// Remove
//===----------------------------------------------------------------------===//

class NoteTrackRemoveAction final : public UndoableAction
{
public:
    explicit NoteTrackRemoveAction(Project& project) noexcept : project(project) {}
    NoteTrackRemoveAction(Project& project, IdGenerator::Id trackId) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;
private:
    Project& project;

    IdGenerator::Id trackId;
    int numEvents = 0;

    ValueTree serializedTreeItem;
    String trackName;
    int trackChannel;

    JUCE_DECLARE_NON_COPYABLE(NoteTrackRemoveAction)
};


