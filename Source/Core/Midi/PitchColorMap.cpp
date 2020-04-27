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

#include "PitchColorMap.h"

#include "SerializationKeys.h"
#include "Project.h"
#include "ColorMapActions.h"

//__________________________________________________________________________
//                                                                          |\
// PitchColorMapEntry                                                       | |
//__________________________________________________________________________| |
//___________________________________________________________________________\|

PitchColorMapEntry::PitchColorMapEntry() noexcept :
    colorMap(nullptr),
    name("0"),
    value(0),
    color(Colours::grey)
{
    id = IdGenerator::generateId();
}

PitchColorMapEntry::PitchColorMapEntry(const PitchColorMapEntry& other) noexcept
{
    colorMap = other.colorMap;
    id = other.id;
    name = other.name;
    value = other.value;
    color = other.color;
    defaultKeys = other.defaultKeys;
}
PitchColorMapEntry& PitchColorMapEntry::operator=(const PitchColorMapEntry& other)
{
    colorMap = other.colorMap;
    id = other.id;
    name = other.name;
    value = other.value;
    color = other.color;
    defaultKeys = other.defaultKeys;
    return *this;
}

PitchColorMapEntry::PitchColorMapEntry(WeakReference<PitchColorMap> owner,
    const PitchColorMapEntry& parametersToCopy) noexcept :
    colorMap(owner),
    id(parametersToCopy.id),
    name(parametersToCopy.name),
    value(parametersToCopy.value),
    color(parametersToCopy.color),
    defaultKeys(parametersToCopy.defaultKeys) {}


PitchColorMapEntry::PitchColorMapEntry(WeakReference<PitchColorMap> owner, String name,
    int value, Colour color, const std::unordered_set<int>& defaultSet) noexcept :
    colorMap(owner), name(name), value(value), color(color), defaultKeys(defaultSet)
{
    id = IdGenerator::generateId();
}

PitchColorMapEntry::PitchColorMapEntry(WeakReference<PitchColorMap> owner, String name,
    int value, Colour color) noexcept :
    colorMap(owner), name(name), value(value), color(color)
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

PitchColorMapEntry PitchColorMapEntry::withDefaultKeys(const std::unordered_set<int>& newKeys)
{
    PitchColorMapEntry e(*this);
    e.defaultKeys = newKeys;
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
std::unordered_set<int> PitchColorMapEntry::getDefaultKeys() const noexcept
{
    return defaultKeys;
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
    tree.setProperty(PitchColor::defaultKeys, defaultKeysToString(), nullptr);
    return tree;
}

void PitchColorMapEntry::deserialize(const ValueTree& tree) noexcept
{
    reset();
    using namespace Serialization;
    name = tree.getProperty(PitchColor::name, "0");
    value = tree.getProperty(PitchColor::value, 0);
    color = Colour::fromString((StringRef)tree.getProperty(PitchColor::color, "ffafafaf"));
    defaultKeys = stringToDefaultKeys(tree.getProperty(PitchColor::defaultKeys, ""));
}

void PitchColorMapEntry::reset() noexcept
{
    defaultKeys.clear();
}

//===------------------------------------------------------------------===//
// Helpers
//===------------------------------------------------------------------===//
void PitchColorMapEntry::applyChanges(const PitchColorMapEntry& other) noexcept
{
    jassert(id == other.id);
    name = other.name;
    value = other.value;
    color = other.color;
    defaultKeys = other.defaultKeys;
}

String PitchColorMapEntry::defaultKeysToNoteNames() const noexcept
{
    Array<int> tmp;
    for (auto k : defaultKeys)
        tmp.add(k % 12);
    tmp.sort();
    StringArray tmp2;
    for (auto k : tmp)
        tmp2.add(MidiMessage::getMidiNoteName(k, true, false, 4));
    return tmp2.joinIntoString(", ");
}

bool PitchColorMapEntry::equalWithoutId(const PitchColorMapEntry* const first, const PitchColorMapEntry* const second) noexcept
{
    return first->value == second->value && first->name == second->name &&
        first->color == second->color && first->defaultKeysToString() == second->defaultKeysToString();
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

    const int defaultKeyResult = first->defaultKeysToString().compare(second->defaultKeysToString());
    if (defaultKeyResult != 0) { return defaultKeyResult; }

    const int idDiff = first->id - second->id;
    const int idResult = (idDiff > 0) - (idDiff < 0);
    return idResult;
}

String PitchColorMapEntry::defaultKeysToString() const noexcept
{
    StringArray tmp;
    for (auto k : defaultKeys)
        tmp.add(String(k % 12));
    tmp.sort(true);
    return tmp.joinIntoString(" ");
}

std::unordered_set<int> PitchColorMapEntry::stringToDefaultKeys(const String& str) const noexcept
{
    std::unordered_set<int> ret;
    auto tmp = StringArray::fromTokens(str.retainCharacters("0123456789 "), false);
    for (auto& s : tmp)
        ret.insert(s.getIntValue() % 12);
    return ret;
}

//__________________________________________________________________________
//                                                                          |\
// PitchColorMap                                                            | |
//__________________________________________________________________________| |
//___________________________________________________________________________\|

PitchColorMap::PitchColorMap() noexcept :
    project(nullptr),
    name("---INIT---")
{
    id = IdGenerator::generateId();
    insert(PitchColorMapEntry(this, "0", 0, Colours::grey), false);
}

PitchColorMap::PitchColorMap(WeakReference<Project> project) noexcept :
    project(project),
    name("---INIT---")
{
    id = IdGenerator::generateId();
    insert(PitchColorMapEntry(this, "0", 0, Colours::grey), false);
}

PitchColorMap::PitchColorMap(WeakReference<Project> project, String name, Array<PitchColorMapEntry>& entries) noexcept :
    project(project), name(name)
{
    id = IdGenerator::generateId();
    insertGroup(entries, true);
    if (!usedNames.contains("0"))
        insert(PitchColorMapEntry(this, "0", 0, Colours::grey), false);
}

//===------------------------------------------------------------------===//
// Accessors
//===------------------------------------------------------------------===//
IdGenerator::Id PitchColorMap::getId() const noexcept
{
    return id;
}

String PitchColorMap::getName() const noexcept
{
    return name;
}

void PitchColorMap::setName(String newName, bool undoable)
{
    name = newName;
    if (project)
        project->broadcastChangePitchColorMap(this);
}

PitchColorMapEntry* PitchColorMap::findEntryById(IdGenerator::Id entryId)
{
    for (auto* entry : collection)
        if (entry->getEntryId() == entryId)
            return entry;
    return nullptr;
}

PitchColorMapEntry* PitchColorMap::findEntryByName(String name)
{
    for (auto* entry : collection)
        if (entry->getName() == name)
            return entry;
    return nullptr;
}

String PitchColorMap::findDefaultColorForKey(int keynum) const noexcept
{
    for (auto* entry : collection)
        if (entry->getDefaultKeys().contains(keynum % 12))
            return entry->getName();
    return "0";
}

Array<PitchColorMapEntry> PitchColorMap::getAllEntries() const noexcept
{
    Array<PitchColorMapEntry> result;
    for (auto* entry : collection)
        result.add(PitchColorMapEntry(*entry));
    return result;
}

std::unordered_set<int>& PitchColorMap::getUsedNotes() noexcept
{
    return usedNotes;
}

bool PitchColorMap::hasNamedUsed(String name)
{
    return usedNames.contains(name);
}

//===------------------------------------------------------------------===//
// Undoable editing
//===------------------------------------------------------------------===//
PitchColorMapEntry* PitchColorMap::insert(const PitchColorMapEntry& entry, bool undoable)
{
    if (usedNames.contains(entry.getName()))
        return nullptr;

    if (undoable && project != nullptr)
    {
        project->getUndoManager().
            perform(new PitchColorMapEntryInsertAction(*this, entry));
    }
    else
    {
        const auto ownedEntry = new PitchColorMapEntry(this, entry);
        collection.addSorted(*ownedEntry, ownedEntry);
        usedNames.insert(ownedEntry->getName());
        for (auto k : ownedEntry->getDefaultKeys())
            usedNotes.insert(k);
        if (project)
        {
            project->broadcastAddPitchColorMapEntry(*ownedEntry);
            project->broadcastPostAddPitchColorMapEntry();
        }
        return ownedEntry;
    }

    return nullptr;
}

bool PitchColorMap::remove(const PitchColorMapEntry& entry, bool undoable)
{
    if (undoable && project != nullptr)
    {
        project->getUndoManager().
            perform(new PitchColorMapEntryRemoveAction(*this, entry));
        return true;
    }
    else
    {
        const int index = collection.indexOfSorted(entry, &entry);
        jassert(index >= 0);
        if (index >= 0)
        {
            auto* removedEntry = collection.getUnchecked(index);
            if (project)
                project->broadcastRemovePitchColorMapEntry(*removedEntry);
            usedNames.erase(entry.getName());
            for (auto k : entry.getDefaultKeys())
                usedNotes.erase(k);
            collection.remove(index, true);
            if (project)
                project->broadcastPostRemovePitchColorMapEntry();
            return true;
        }
        return false;
    }
}

bool PitchColorMap::change(const PitchColorMapEntry& oldEntry, const PitchColorMapEntry& newEntry, bool undoable)
{
    if (oldEntry.getName() != newEntry.getName() && usedNames.contains(newEntry.getName()))
        return false;
    if (undoable && project != nullptr)
    {
        project->getUndoManager().
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
            usedNames.erase(oldEntry.getName());
            for (auto k : oldEntry.getDefaultKeys())
                usedNotes.erase(k);
            collection.remove(index, false);
            collection.addSorted(*changedEntry, changedEntry);
            usedNames.insert(newEntry.getName());
            for (auto k : newEntry.getDefaultKeys())
                usedNotes.insert(k);
            if (project)
            {
                project->broadcastChangePitchColorMapEntry(oldEntry, *changedEntry);
                project->broadcastPostChangePitchColorMapEntry();
            }
            return true;
        }

        return false;
    }

    return true;
}

bool PitchColorMap::insertGroup(Array<PitchColorMapEntry>& entries, bool undoable)
{
    if (undoable && project != nullptr)
    {
        project->getUndoManager().
            perform(new PitchColorMapEntrysGroupInsertAction(*this, entries));
        return true;
    }
    else
    {
        bool added = false;
        for (int i = 0; i < entries.size(); ++i)
        {
            const PitchColorMapEntry& entryParams = entries.getUnchecked(i);
            if (usedNames.contains(entryParams.getName()))
                continue;
            const auto ownedEntry = new PitchColorMapEntry(this, entryParams);
            collection.addSorted(*ownedEntry, ownedEntry);
            usedNames.insert(ownedEntry->getName());
            for (auto k : ownedEntry->getDefaultKeys())
                usedNotes.insert(k);
            if (project)
                project->broadcastAddPitchColorMapEntry(*ownedEntry);
            added = true;
        }
        if (project)
            project->broadcastPostAddPitchColorMapEntry();
        return added;
    }
}

bool PitchColorMap::removeGroup(Array<PitchColorMapEntry>& entries, bool undoable)
{
    if (undoable && project != nullptr)
    {
        project->getUndoManager().
            perform(new PitchColorMapEntrysGroupRemoveAction(*this, entries));
        return true;
    }
    else
    {
        bool deleted = false;
        for (int i = 0; i < entries.size(); ++i)
        {
            const PitchColorMapEntry& entry = entries.getUnchecked(i);
            const int index = collection.indexOfSorted(entry, &entry);
            jassert(index >= 0);
            if (index >= 0)
            {
                auto* removedEntry = collection.getUnchecked(index);
                if (project)
                    project->broadcastRemovePitchColorMapEntry(*removedEntry);
                usedNames.erase(removedEntry->getName());
                for (auto k : removedEntry->getDefaultKeys())
                    usedNotes.erase(k);
                collection.remove(index, true);
                deleted = true;
            }
        }
        if (project)
            project->broadcastPostRemovePitchColorMapEntry();
        return deleted;
    }
}

bool PitchColorMap::changeGroup(Array<PitchColorMapEntry>& entriesBefore, 
    Array<PitchColorMapEntry>& entriesAfter, bool undoable)
{
    jassert(entriesBefore.size() == entriesAfter.size());

    if (undoable && project != nullptr)
    {
        project->getUndoManager().
            perform(new PitchColorMapEntrysGroupChangeAction(*this, entriesBefore, entriesAfter));
        return true;
    }
    else
    {
        bool changed = false;
        for (int i = 0; i < entriesBefore.size(); ++i)
        {
            const PitchColorMapEntry& oldParams = entriesBefore.getReference(i);
            const PitchColorMapEntry& newParams = entriesAfter.getReference(i);

            if (oldParams.getName() != newParams.getName() && usedNames.contains(newParams.getName()))
                continue;

            const int index = collection.indexOfSorted(oldParams, &oldParams);

            jassert(index >= 0);
            if (index >= 0)
            {
                auto* changedEntry = static_cast<PitchColorMapEntry*>(collection.getUnchecked(index));
                changedEntry->applyChanges(newParams);
                usedNames.erase(oldParams.getName());
                for (auto k : oldParams.getDefaultKeys())
                    usedNotes.erase(k);
                collection.remove(index, false);
                collection.addSorted(*changedEntry, changedEntry);
                usedNames.insert(newParams.getName());
                for (auto k : newParams.getDefaultKeys())
                    usedNotes.insert(k);
                if (project)
                    project->broadcastChangePitchColorMapEntry(oldParams, *changedEntry);
                changed = true;
            }
        }
        if (project)
            project->broadcastPostChangePitchColorMapEntry();
        return changed;
    }
}

//===------------------------------------------------------------------===//
// Serializable
//===------------------------------------------------------------------===//
ValueTree PitchColorMap::serialize() const
{
    ValueTree tree(Serialization::PitchColor::colorMap);
    tree.setProperty(Serialization::PitchColor::name, name, nullptr);
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

    name = tree.getProperty(Serialization::PitchColor::name).toString();
    for (const auto& e : root)
    {
        if (e.hasType(Serialization::PitchColor::entry))
        {
            auto* entry = new PitchColorMapEntry(this);
            entry->deserialize(e);

            if (!usedNames.contains(entry->getName()))
            {
                collection.addSorted(*entry, entry);
                usedNames.insert(entry->getName());
                for (auto k : entry->getDefaultKeys())
                    usedNotes.insert(k);
            }
        }
    }

    sort();
}

void PitchColorMap::reset()
{
    name = "---INIT---";
    collection.clear();
    usedNames.clear();
    usedNotes.clear();
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

//__________________________________________________________________________
//                                                                          |\
// PitchColorMapNameComparator                                              | |
//__________________________________________________________________________| |
//___________________________________________________________________________\|

int PitchColorMapNameComparator::compareElements(const PitchColorMap* const first, const PitchColorMap* const second) noexcept
{
    return first->name.compare(second->name);
}

