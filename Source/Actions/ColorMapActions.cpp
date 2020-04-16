/*
  ==============================================================================

    ColorMapActions.cpp
    Created: 12 Apr 2020 8:25:37pm
    Author:  bowen

  ==============================================================================
*/

#include "ColorMapActions.h"

#include "Project.h"

//===----------------------------------------------------------------------===//
// Insert
//===----------------------------------------------------------------------===//

PitchColorMapEntryInsertAction::PitchColorMapEntryInsertAction(PitchColorMap& colorMap,
    const PitchColorMapEntry& entry) noexcept :
    colorMap(colorMap),
    
    entry(entry) {}

bool PitchColorMapEntryInsertAction::perform()
{
    return (colorMap.insert(entry, false) != nullptr);
}

bool PitchColorMapEntryInsertAction::undo()
{
    return colorMap.remove(entry, false);
}

int PitchColorMapEntryInsertAction::getSizeInUnits()
{
    return sizeof(PitchColorMapEntry);
}

//===----------------------------------------------------------------------===//
// Remove
//===----------------------------------------------------------------------===//

PitchColorMapEntryRemoveAction::PitchColorMapEntryRemoveAction(PitchColorMap& colorMap,
    const PitchColorMapEntry& entry) noexcept :
    colorMap(colorMap),
    entry(entry) {}

bool PitchColorMapEntryRemoveAction::perform()
{
    return colorMap.remove(entry, false);
}

bool PitchColorMapEntryRemoveAction::undo()
{
    return (colorMap.insert(entry, false) != nullptr);
}

int PitchColorMapEntryRemoveAction::getSizeInUnits()
{
    return sizeof(PitchColorMapEntry);
}

//===----------------------------------------------------------------------===//
// Change
//===----------------------------------------------------------------------===//

PitchColorMapEntryChangeAction::PitchColorMapEntryChangeAction(PitchColorMap& colorMap,
    const PitchColorMapEntry& entry, const PitchColorMapEntry& newParameters) noexcept :
    colorMap(colorMap),
    entryBefore(entry),
    entryAfter(newParameters)
{
    jassert(entry.getEntryId() == newParameters.getEntryId());
}

bool PitchColorMapEntryChangeAction::perform()
{
    return colorMap.change(entryBefore, entryAfter, false);
}

bool PitchColorMapEntryChangeAction::undo()
{
    return colorMap.change(entryAfter, entryBefore, false);
}

int PitchColorMapEntryChangeAction::getSizeInUnits()
{
    return sizeof(PitchColorMapEntry) * 2;
}

UndoableAction* PitchColorMapEntryChangeAction::createCoalescedAction(UndoableAction* nextAction)
{
    if (PitchColorMapEntryChangeAction* nextChanger =
        dynamic_cast<PitchColorMapEntryChangeAction*>(nextAction))
    {
        const bool idsAreEqual = entryBefore.getEntryId() == nextChanger->entryAfter.getEntryId();

        if (idsAreEqual)
        {
            return new PitchColorMapEntryChangeAction(colorMap,
                entryBefore, nextChanger->entryAfter);
        }
    }

    (void)nextAction;
    return nullptr;
}

//===----------------------------------------------------------------------===//
// Insert Group
//===----------------------------------------------------------------------===//

PitchColorMapEntrysGroupInsertAction::PitchColorMapEntrysGroupInsertAction(PitchColorMap& colorMap,
    Array<PitchColorMapEntry>& target) noexcept :
    colorMap(colorMap)
{
    entries.swapWith(target);
}

bool PitchColorMapEntrysGroupInsertAction::perform()
{
    return colorMap.insertGroup(entries, false);
}

bool PitchColorMapEntrysGroupInsertAction::undo()
{
    return colorMap.removeGroup(entries, false);
}

int PitchColorMapEntrysGroupInsertAction::getSizeInUnits()
{
    return (sizeof(PitchColorMapEntry) * entries.size());
}

//===----------------------------------------------------------------------===//
// Remove Group
//===----------------------------------------------------------------------===//

PitchColorMapEntrysGroupRemoveAction::PitchColorMapEntrysGroupRemoveAction(PitchColorMap& colorMap,
    Array<PitchColorMapEntry>& target) noexcept :
    colorMap(colorMap)
{
    entries.swapWith(target);
}

bool PitchColorMapEntrysGroupRemoveAction::perform()
{
    return colorMap.removeGroup(entries, false);
}

bool PitchColorMapEntrysGroupRemoveAction::undo()
{
    return colorMap.insertGroup(entries, false);
}

int PitchColorMapEntrysGroupRemoveAction::getSizeInUnits()
{
    return (sizeof(PitchColorMapEntry) * entries.size());
}

//===----------------------------------------------------------------------===//
// Change Group
//===----------------------------------------------------------------------===//

PitchColorMapEntrysGroupChangeAction::PitchColorMapEntrysGroupChangeAction(PitchColorMap& colorMap,
    Array<PitchColorMapEntry>& state1, Array<PitchColorMapEntry>& state2) noexcept :
    colorMap(colorMap)
{
    entriesBefore.swapWith(state1);
    entriesAfter.swapWith(state2);
}

bool PitchColorMapEntrysGroupChangeAction::perform()
{
    return colorMap.changeGroup(entriesBefore, entriesAfter, false);
}

bool PitchColorMapEntrysGroupChangeAction::undo()
{
    return colorMap.changeGroup(entriesAfter, entriesBefore, false);
}

int PitchColorMapEntrysGroupChangeAction::getSizeInUnits()
{
    return (sizeof(PitchColorMapEntry) * entriesBefore.size()) +
        (sizeof(PitchColorMapEntry) * entriesAfter.size());
}

UndoableAction* PitchColorMapEntrysGroupChangeAction::createCoalescedAction(UndoableAction* nextAction)
{
    if (PitchColorMapEntrysGroupChangeAction* nextChanger =
        dynamic_cast<PitchColorMapEntrysGroupChangeAction*>(nextAction))
    {
        if (entriesBefore.size() != nextChanger->entriesAfter.size())
        {
            return nullptr;
        }

        for (int i = 0; i < entriesBefore.size(); ++i)
        {
            if (entriesBefore.getUnchecked(i).getEntryId() !=
                nextChanger->entriesAfter.getUnchecked(i).getEntryId())
            {
                return nullptr;
            }
        }

        return new PitchColorMapEntrysGroupChangeAction(colorMap,
            entriesBefore, nextChanger->entriesAfter);
    }

    (void)nextAction;
    return nullptr;
}
