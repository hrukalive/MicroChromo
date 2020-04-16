/*
  ==============================================================================

    NoteTrackActions.h
    Created: 14 Apr 2020 6:28:06pm
    Author:  bowen

  ==============================================================================
*/

#pragma once

#include "Common.h"
#include "NoteTrack.h"

class Project;

//===----------------------------------------------------------------------===//
// Rename
//===----------------------------------------------------------------------===//
class MidiTrackRenameAction final : public UndoableAction
{
public:
    explicit MidiTrackRenameAction(Project& project) noexcept : project(project) {}
    MidiTrackRenameAction(Project& project, IdGenerator::Id trackId, const String& name) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;

private:
    Project& project;
    IdGenerator::Id trackId;

    String nameBefore;
    String nameAfter;

    JUCE_DECLARE_NON_COPYABLE(MidiTrackRenameAction)
};

//===----------------------------------------------------------------------===//
// Channel
//===----------------------------------------------------------------------===//
class MidiTrackChannelChangeAction final : public UndoableAction
{
public:
    explicit MidiTrackChannelChangeAction(Project& project) noexcept : project(project) {}
    MidiTrackChannelChangeAction(Project& project, IdGenerator::Id trackId, int channel) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;

private:
    Project& project;
    IdGenerator::Id trackId;

    int channelBefore;
    int channelAfter;

    JUCE_DECLARE_NON_COPYABLE(MidiTrackChannelChangeAction)
};
