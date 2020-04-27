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

#pragma once

#include "Common.h"

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


