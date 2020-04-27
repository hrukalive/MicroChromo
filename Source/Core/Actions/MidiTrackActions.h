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
