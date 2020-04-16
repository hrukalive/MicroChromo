/*
  ==============================================================================

    ColorMapActions.h
    Created: 12 Apr 2020 8:25:37pm
    Author:  bowen

  ==============================================================================
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
