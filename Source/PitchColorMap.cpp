/*
  ==============================================================================

    PitchColorMap.cpp
    Created: 14 Apr 2020 7:04:35pm
    Author:  bowen

  ==============================================================================
*/

#include "PitchColorMap.h"
#include "SerializationKeys.h"
#include "ColorMapActions.h"
#include "Project.h"

//==============================================================================
PitchColorMapEntry::PitchColorMapEntry() noexcept :
    colorMap(nullptr),
    name("0"),
    value(0),
    color(Colours::grey),
    defaultKey(-1)
{
    id = IdGenerator::generateId();
}

PitchColorMapEntry::PitchColorMapEntry(WeakReference<PitchColorMap> owner,
    const PitchColorMapEntry& parametersToCopy) noexcept :
    colorMap(owner),
    id(parametersToCopy.id),
    name(parametersToCopy.name),
    value(parametersToCopy.value),
    color(parametersToCopy.color),
    defaultKey(parametersToCopy.defaultKey) {}

PitchColorMapEntry::PitchColorMapEntry(WeakReference<PitchColorMap> owner, String name,
    int value, Colour color, int defaultKey) noexcept :
    colorMap(owner), name(name), value(value), color(color), defaultKey(defaultKey)
{
    id = IdGenerator::generateId();
}

PitchColorMapEntry PitchColorMapEntry::withName(const String newName)
{
    PitchColorMapEntry e(*this);
    e.name = newName;
    return e;
}

PitchColorMapEntry PitchColorMapEntry::withValue(const int newValue)
{
    PitchColorMapEntry e(*this);
    e.value = newValue;
    return e;
}

PitchColorMapEntry PitchColorMapEntry::withColor(const Colour& newColor)
{
    PitchColorMapEntry e(*this);
    e.color = newColor;
    return e;
}

PitchColorMapEntry PitchColorMapEntry::withDefaultKey(const int newKey)
{
    PitchColorMapEntry e(*this);
    e.defaultKey = newKey;
    return e;
}

//===------------------------------------------------------------------===//
// Accessors
//===------------------------------------------------------------------===//
IdGenerator::Id PitchColorMapEntry::getEntryId() const noexcept
{
    return id;
}
String PitchColorMapEntry::getName() const noexcept
{
    return name;
}
int PitchColorMapEntry::getValue() const noexcept
{
    return value;
}
Colour PitchColorMapEntry::getColor() const noexcept
{
    return color;
}
int PitchColorMapEntry::getDefaultKey() const noexcept
{
    return defaultKey;
}
PitchColorMap* PitchColorMapEntry::getPitchColorMap()
{
    jassert(colorMap);
    return colorMap;
}

//===------------------------------------------------------------------===//
// Serializable
//===------------------------------------------------------------------===//
ValueTree PitchColorMapEntry::serialize() const noexcept
{
    using namespace Serialization;
    ValueTree tree(PitchColor::entry);
    tree.setProperty(PitchColor::name, name, nullptr);
    tree.setProperty(PitchColor::value, value, nullptr);
    tree.setProperty(PitchColor::color, color.toString(), nullptr);
    tree.setProperty(PitchColor::defaultKey, defaultKey, nullptr);
    return tree;
}

void PitchColorMapEntry::deserialize(const ValueTree& tree) noexcept
{
    reset();
    using namespace Serialization;
    name = tree.getProperty(PitchColor::name);
    value = tree.getProperty(PitchColor::value);
    color = Colour::fromString((StringRef)tree.getProperty(PitchColor::color));
    defaultKey = tree.getProperty(PitchColor::defaultKey);
}

void PitchColorMapEntry::reset() noexcept {}

//===------------------------------------------------------------------===//
// Helpers
//===------------------------------------------------------------------===//

void PitchColorMapEntry::applyChanges(const PitchColorMapEntry& other) noexcept
{
    jassert(id == other.id);
    name = other.name;
    value = other.value;
    color = other.color;
    defaultKey = other.defaultKey;
}

int PitchColorMapEntry::compareElements(const PitchColorMapEntry* const first, const PitchColorMapEntry* const second) noexcept
{
    if (first == second) { return 0; }

    const float valueDiff = first->value - second->value;
    const int valueResult = (valueDiff > 0.f) - (valueDiff < 0.f);
    if (valueResult != 0) { return valueResult; }

    const int nameResult = first->name.compare(second->name);
    if (nameResult != 0) { return nameResult; }

    const int colorResult = first->color.toString().compare(second->color.toString());
    if (colorResult != 0) { return colorResult; }

    const float defaultKeyDiff = first->defaultKey - second->defaultKey;
    const int defaultKeyResult = (defaultKeyDiff > 0.f) - (defaultKeyDiff < 0.f);
    if (defaultKeyResult != 0) { return defaultKeyResult; }

    const int idDiff = first->id - second->id;
    const int idResult = (idDiff > 0) - (idDiff < 0);
    return idResult;
}


//==============================================================================
PitchColorMap::PitchColorMap(Project& project) noexcept :
    project(project),
    name("---INIT---")
{
    insert(PitchColorMapEntry(this, "0", 0, Colours::grey), false);
}

PitchColorMap::PitchColorMap(Project& project, String name, const Array<PitchColorMapEntry>& entries) noexcept :
    project(project), name(name)
{
    for (auto& entry : entries)
        insert(entry, false);
    if (!usedNames.contains("0"))
        insert(PitchColorMapEntry(this, "0", 0, Colours::grey), false);
}

//===------------------------------------------------------------------===//
// Accessors
//===------------------------------------------------------------------===//
PitchColorMapEntry* PitchColorMap::getEntryById(IdGenerator::Id entryId)
{
    for (auto* entry : collection)
        if (entry->getEntryId() == entryId)
            return entry;
    return nullptr;
}

Array<PitchColorMapEntry> PitchColorMap::getAllEntries()
{
    Array<PitchColorMapEntry> result;
    for (auto* entry : collection)
        result.add(PitchColorMapEntry(*entry));
    return result;
}

//===------------------------------------------------------------------===//
// Undoable editing
//===------------------------------------------------------------------===//
PitchColorMapEntry* PitchColorMap::insert(const PitchColorMapEntry& entry, bool undoable)
{
    if (usedNames.contains(entry.getName()))
        return nullptr;

    if (undoable)
    {
        project.getUndoManager().
            perform(new PitchColorMapEntryInsertAction(*this, entry));
    }
    else
    {
        const auto ownedEntry = new PitchColorMapEntry(this, entry);
        collection.addSorted(*ownedEntry, ownedEntry);
        project.broadcastAddPitchColorMapEntry(*ownedEntry);
        project.broadcastPostAddPitchColorMapEntry();
        return ownedEntry;
    }

    return nullptr;
}
bool PitchColorMap::remove(const PitchColorMapEntry& entry, bool undoable)
{
    if (undoable)
    {
        project.getUndoManager().
            perform(new PitchColorMapEntryRemoveAction(*this, entry));
    }
    else
    {
        const int index = collection.indexOfSorted(entry, &entry);
        jassert(index >= 0);
        if (index >= 0)
        {
            auto* removedEntry = collection.getUnchecked(index);
            project.broadcastRemovePitchColorMapEntry(*removedEntry);
            collection.remove(index, true);
            usedNames.erase(entry.getName());
            project.broadcastPostRemovePitchColorMapEntry();
            return true;
        }

        return false;
    }

    return true;
}
bool PitchColorMap::change(const PitchColorMapEntry& oldEntry, const PitchColorMapEntry& newEntry, bool undoable)
{
    if (usedNames.contains(newEntry.getName()))
        return nullptr;
    if (undoable)
    {
        project.getUndoManager().
            perform(new PitchColorMapEntryChangeAction(*this, oldEntry, newEntry));
    }
    else
    {
        const int index = collection.indexOfSorted(oldEntry, &oldEntry);
        jassert(index >= 0);
        if (index >= 0)
        {
            auto* changedEntry = static_cast<PitchColorMapEntry*>(collection.getUnchecked(index));
            changedEntry->applyChanges(newEntry);
            collection.remove(index, false);
            usedNames.erase(oldEntry.getName());
            collection.addSorted(*changedEntry, changedEntry);
            usedNames.insert(newEntry.getName());
            project.broadcastChangePitchColorMapEntry(oldEntry, *changedEntry);
            project.broadcastPostChangePitchColorMapEntry();
            return true;
        }

        return false;
    }

    return true;
}

bool PitchColorMap::insertGroup(Array<PitchColorMapEntry>& entries, bool undoable)
{
    if (undoable)
    {
        project.getUndoManager().
            perform(new PitchColorMapEntrysGroupInsertAction(*this, entries));
    }
    else
    {
        for (int i = 0; i < entries.size(); ++i)
        {
            const PitchColorMapEntry& entryParams = entries.getUnchecked(i);
            if (usedNames.contains(entryParams.getName()))
                continue;
            const auto ownedEntry = new PitchColorMapEntry(this, entryParams);
            collection.addSorted(*ownedEntry, ownedEntry);
            usedNames.insert(ownedEntry->getName());
            project.broadcastAddPitchColorMapEntry(*ownedEntry);
        }
        project.broadcastPostAddPitchColorMapEntry();
    }

    return true;
}

bool PitchColorMap::removeGroup(Array<PitchColorMapEntry>& entries, bool undoable)
{
    if (undoable)
    {
        project.getUndoManager().
            perform(new PitchColorMapEntrysGroupRemoveAction(*this, entries));
    }
    else
    {
        for (int i = 0; i < entries.size(); ++i)
        {
            const PitchColorMapEntry& note = entries.getUnchecked(i);
            const int index = collection.indexOfSorted(note, &note);
            jassert(index >= 0);
            if (index >= 0)
            {
                auto* removedEntry = collection.getUnchecked(index);
                project.broadcastRemovePitchColorMapEntry(*removedEntry);
                collection.remove(index, true);
                usedNames.erase(removedEntry->getName());
            }
        }
        project.broadcastPostRemovePitchColorMapEntry();
    }

    return true;
}

bool PitchColorMap::changeGroup(Array<PitchColorMapEntry>& entriesBefore, 
    Array<PitchColorMapEntry>& entriesAfter, bool undoable)
{
    jassert(entriesBefore.size() == entriesAfter.size());

    if (undoable)
    {
        project.getUndoManager().
            perform(new PitchColorMapEntrysGroupChangeAction(*this, entriesBefore, entriesAfter));
    }
    else
    {
        for (int i = 0; i < entriesBefore.size(); ++i)
        {
            const PitchColorMapEntry& oldParams = entriesBefore.getReference(i);
            const PitchColorMapEntry& newParams = entriesAfter.getReference(i);

            if (usedNames.contains(newParams.getName()))
                return nullptr;

            const int index = collection.indexOfSorted(oldParams, &oldParams);

            jassert(index >= 0);
            if (index >= 0)
            {
                auto* changedEntry = static_cast<PitchColorMapEntry*>(collection.getUnchecked(index));
                changedEntry->applyChanges(newParams);
                collection.remove(index, false);
                usedNames.erase(oldParams.getName());
                collection.addSorted(*changedEntry, changedEntry);
                usedNames.insert(newParams.getName());
                project.broadcastChangePitchColorMapEntry(oldParams, *changedEntry);
            }
        }
        project.broadcastPostChangePitchColorMapEntry();
    }

    return true;
}

//===------------------------------------------------------------------===//
// Serializable
//===------------------------------------------------------------------===//
ValueTree PitchColorMap::serialize() const
{
    ValueTree tree(Serialization::PitchColor::colorMap);

    for (int i = 0; i < collection.size(); ++i)
    {
        const PitchColorMapEntry* entry = collection.getUnchecked(i);
        tree.appendChild(entry->serialize(), nullptr);
    }

    return tree;
}

void PitchColorMap::deserialize(const ValueTree& tree)
{
    reset();

    const auto root =
        tree.hasType(Serialization::PitchColor::colorMap) ?
        tree : tree.getChildWithName(Serialization::PitchColor::colorMap);

    if (!root.isValid())
        return;

    for (const auto& e : root)
    {
        if (e.hasType(Serialization::PitchColor::entry))
        {
            auto* entry = new PitchColorMapEntry(this);
            entry->deserialize(e);

            if (!usedNames.contains(entry->getName()))
            {
                collection.add(entry);
                usedNames.insert(entry->getName());
            }
        }
    }

    sort();
}

void PitchColorMap::reset()
{
    collection.clear();
    usedNames.clear();
}

//===------------------------------------------------------------------===//
// OwnedArray wrapper
//===------------------------------------------------------------------===//
void PitchColorMap::sort()
{
    if (collection.size() > 0)
    {
        collection.sort(*collection.getFirst(), true);
    }
}


//==============================================================================
int PitchColorMapNameComparator::compareElements(const PitchColorMap& first, const PitchColorMap& second)
{
    return first.name.compare(second.name);
}

