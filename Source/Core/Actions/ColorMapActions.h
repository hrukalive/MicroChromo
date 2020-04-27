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
#include "PitchColorMap.h"

//===----------------------------------------------------------------------===//
// Insert
//===----------------------------------------------------------------------===//
class PitchColorMapEntryInsertAction final : public UndoableAction
{
public:
    explicit PitchColorMapEntryInsertAction(PitchColorMap& colorMap) noexcept : colorMap(colorMap) {}
    PitchColorMapEntryInsertAction(PitchColorMap& colorMap, const PitchColorMapEntry& target) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;

private:
    PitchColorMap& colorMap;
    PitchColorMapEntry entry;

    JUCE_DECLARE_NON_COPYABLE(PitchColorMapEntryInsertAction)
};

//===----------------------------------------------------------------------===//
// Remove
//===----------------------------------------------------------------------===//
class PitchColorMapEntryRemoveAction final : public UndoableAction
{
public:
    explicit PitchColorMapEntryRemoveAction(PitchColorMap& colorMap) noexcept : colorMap(colorMap) {}
    PitchColorMapEntryRemoveAction(PitchColorMap& colorMap, const PitchColorMapEntry& target) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;

private:
    PitchColorMap& colorMap;
    PitchColorMapEntry entry;

    JUCE_DECLARE_NON_COPYABLE(PitchColorMapEntryRemoveAction)
};

//===----------------------------------------------------------------------===//
// Change
//===----------------------------------------------------------------------===//
class PitchColorMapEntryChangeAction final : public UndoableAction
{
public:
    explicit PitchColorMapEntryChangeAction(PitchColorMap& colorMap) noexcept : colorMap(colorMap) {}
    PitchColorMapEntryChangeAction(PitchColorMap& colorMap,
        const PitchColorMapEntry& entry, const PitchColorMapEntry& newParameters) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;
    UndoableAction* createCoalescedAction(UndoableAction* nextAction) override;

private:
    PitchColorMap& colorMap;

    PitchColorMapEntry entryBefore;
    PitchColorMapEntry entryAfter;

    JUCE_DECLARE_NON_COPYABLE(PitchColorMapEntryChangeAction)
};

//===----------------------------------------------------------------------===//
// Insert Group
//===----------------------------------------------------------------------===//
class PitchColorMapEntrysGroupInsertAction final : public UndoableAction
{
public:
    explicit PitchColorMapEntrysGroupInsertAction(PitchColorMap& colorMap) noexcept : colorMap(colorMap) {}
    PitchColorMapEntrysGroupInsertAction(PitchColorMap& colorMap, Array<PitchColorMapEntry>& target) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;

private:
    PitchColorMap& colorMap;
    Array<PitchColorMapEntry> entries;

    JUCE_DECLARE_NON_COPYABLE(PitchColorMapEntrysGroupInsertAction)
};

//===----------------------------------------------------------------------===//
// Remove Group
//===----------------------------------------------------------------------===//
class PitchColorMapEntrysGroupRemoveAction final : public UndoableAction
{
public:
    explicit PitchColorMapEntrysGroupRemoveAction(PitchColorMap& colorMap) noexcept : colorMap(colorMap) {}
    PitchColorMapEntrysGroupRemoveAction(PitchColorMap& colorMap, Array<PitchColorMapEntry>& target) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;

private:
    PitchColorMap& colorMap;
    Array<PitchColorMapEntry> entries;

    JUCE_DECLARE_NON_COPYABLE(PitchColorMapEntrysGroupRemoveAction)
};

//===----------------------------------------------------------------------===//
// Change Group
//===----------------------------------------------------------------------===//
class PitchColorMapEntrysGroupChangeAction final : public UndoableAction
{
public:

    explicit PitchColorMapEntrysGroupChangeAction(PitchColorMap& colorMap) noexcept : colorMap(colorMap) {}

    PitchColorMapEntrysGroupChangeAction(PitchColorMap& colorMap,
        Array<PitchColorMapEntry>& state1, Array<PitchColorMapEntry>& state2) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;
    UndoableAction* createCoalescedAction(UndoableAction* nextAction) override;

private:
    PitchColorMap& colorMap;

    Array<PitchColorMapEntry> entriesBefore;
    Array<PitchColorMapEntry> entriesAfter;

    JUCE_DECLARE_NON_COPYABLE(PitchColorMapEntrysGroupChangeAction)
};
