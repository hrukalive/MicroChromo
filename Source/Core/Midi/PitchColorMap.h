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
#include "Serializable.h"

class Project;
class PitchColorMap;

//==============================================================================
class PitchColorMapEntry : public Serializable
{
public:
    PitchColorMapEntry() noexcept;

    PitchColorMapEntry(const PitchColorMapEntry& other) noexcept;
    PitchColorMapEntry& operator= (const PitchColorMapEntry & other);

    PitchColorMapEntry(PitchColorMapEntry && other) noexcept = default;
    PitchColorMapEntry& operator= (PitchColorMapEntry && other) noexcept = default;

    PitchColorMapEntry(WeakReference<PitchColorMap> owner, 
        const PitchColorMapEntry& parametersToCopy) noexcept;
    PitchColorMapEntry(WeakReference<PitchColorMap> owner, String name,
        int value, Colour color, const std::unordered_set<int>& defaultSet,
        const std::unordered_set<int>& allowedSet) noexcept;
    explicit PitchColorMapEntry(WeakReference<PitchColorMap> owner, String name = "0", 
        int value = 0, Colour color = Colours::grey) noexcept;

    PitchColorMapEntry withName(const String newName);
    PitchColorMapEntry withValue(const int newValue);
    PitchColorMapEntry withColor(const Colour& newColor);
    PitchColorMapEntry withDefaultKeys(const std::unordered_set<int>& newkeys);
    PitchColorMapEntry withAllowedKeys(const std::unordered_set<int>& newkeys);
    PitchColorMapEntry withParameters(const PitchColorMapEntry& other);

    //===------------------------------------------------------------------===//
    // Accessors
    //===------------------------------------------------------------------===//
    IdGenerator::Id getEntryId() const noexcept;
    String getName() const noexcept;
    int getValue() const noexcept;
    Colour getColor() const noexcept;
    std::unordered_set<int> getDefaultKeys() const noexcept;
    std::unordered_set<int> getAllowedKeys() const noexcept;
    PitchColorMap* getPitchColorMap();
    bool isAllowedForNote(int notenum) const noexcept;

    //===------------------------------------------------------------------===//
    // Serializable
    //===------------------------------------------------------------------===//
    ValueTree serialize() const noexcept override;
    void deserialize(const ValueTree& tree) noexcept override;
    void reset() noexcept override;

    //===------------------------------------------------------------------===//
    // Helpers
    //===------------------------------------------------------------------===//
    void applyChanges(const PitchColorMapEntry& parameters) noexcept;
    String defaultKeysToNoteNames() const noexcept;
    String allowedKeysToNoteNames() const noexcept;

    static inline bool equalWithoutId(const PitchColorMapEntry& first, const PitchColorMapEntry& second) noexcept
    {
        return PitchColorMapEntry::equalWithoutId(&first, &second);
    }

    static bool equalWithoutId(const PitchColorMapEntry* const first, const PitchColorMapEntry* const second) noexcept;

private:
    //===------------------------------------------------------------------===//
    // Helpers
    //===------------------------------------------------------------------===//
    String keysToString(const std::unordered_set<int>& set) const noexcept;
    std::unordered_set<int> stringToKeys(const String& str) const noexcept;

    //==============================================================================
    friend struct PitchColorMapEntryValueComparator;
    friend struct PitchColorMapEntryNameComparator;

    //==============================================================================
    WeakReference<PitchColorMap> colorMap;
    IdGenerator::Id id;
    String name;
    int value;
    Colour color;
    std::unordered_set<int> defaultKeys, allowedKeys;

    //==============================================================================
    JUCE_LEAK_DETECTOR(PitchColorMapEntry)
};

class PitchColorMapEntryComparator
{
public:
    PitchColorMapEntryComparator(bool ascending) : _ascending(ascending) {}

    virtual inline int compareElements(const PitchColorMapEntry& first, const PitchColorMapEntry& second) noexcept
    {
        return compareElements(&first, &second);
    }

    virtual int compareElements(const PitchColorMapEntry* const first, const PitchColorMapEntry* const second) noexcept = 0;

protected:
    bool _ascending = true;
};

struct PitchColorMapEntryValueComparator : public PitchColorMapEntryComparator
{
    PitchColorMapEntryValueComparator(bool ascending) : PitchColorMapEntryComparator(ascending) {}
    int compareElements(const PitchColorMapEntry* const first, const PitchColorMapEntry* const second) noexcept override;
};

struct PitchColorMapEntryNameComparator : public PitchColorMapEntryComparator
{
    PitchColorMapEntryNameComparator(bool ascending) : PitchColorMapEntryComparator(ascending) {}
    int compareElements(const PitchColorMapEntry* const first, const PitchColorMapEntry* const second) noexcept override;
};

//==============================================================================
class PitchColorMap : public Serializable
{
public:
    PitchColorMap() noexcept;

    PitchColorMap(const PitchColorMap& other) noexcept = default;
    PitchColorMap& operator= (const PitchColorMap& other) = default;

    PitchColorMap(PitchColorMap&& other) noexcept = default;
    PitchColorMap& operator= (PitchColorMap&& other) noexcept = default;

    PitchColorMap(WeakReference<Project> project) noexcept;
    PitchColorMap(WeakReference<Project> project, String name, Array<PitchColorMapEntry>& entries) noexcept;

    //===------------------------------------------------------------------===//
    // Accessors
    //===------------------------------------------------------------------===//
    IdGenerator::Id getId() const noexcept;
    String getName() const noexcept;
    void setName(String newName, bool undoable);

    PitchColorMapEntry* findEntryById(IdGenerator::Id entryId);
    PitchColorMapEntry* findEntryByName(String name);
    String findDefaultColorForKey(int keynum) const noexcept;
    Array<PitchColorMapEntry> getAllEntries() const noexcept;
    std::unordered_set<int>& getUsedNotes() noexcept;
    bool hasNamedUsed(String name);

    //===------------------------------------------------------------------===//
    // Undoable editing
    //===------------------------------------------------------------------===//
    PitchColorMapEntry* insert(const PitchColorMapEntry& entry, bool undoable);
    bool remove(const PitchColorMapEntry& entry, bool undoable);
    bool change(const PitchColorMapEntry& oldEntry, const PitchColorMapEntry& newEntry, bool undoable);

    bool insertGroup(Array<PitchColorMapEntry>& entries, bool undoable);
    bool removeGroup(Array<PitchColorMapEntry>& entries, bool undoable);
    bool changeGroup(Array<PitchColorMapEntry>& entriesBefore, Array<PitchColorMapEntry>& entriesAfter, bool undoable);

    void changeSortMethod(bool isByValue, bool ascending);

    //===------------------------------------------------------------------===//
    // Serializable
    //===------------------------------------------------------------------===//
    ValueTree serialize() const override;
    void deserialize(const ValueTree& tree) override;
    void reset() override;

    //===------------------------------------------------------------------===//
    // OwnedArray wrapper
    //===------------------------------------------------------------------===//
    void sort();

    inline int size() const noexcept
    {
        return this->collection.size();
    }

    inline PitchColorMapEntry* const* begin() const noexcept
    {
        return this->collection.begin();
    }

    inline PitchColorMapEntry* const* end() const noexcept
    {
        return this->collection.end();
    }

    inline PitchColorMapEntry* getUnchecked(const int index) const noexcept
    {
        return this->collection.getUnchecked(index);
    }

    inline PitchColorMapEntry* operator[] (int index) const noexcept
    {
        return collection[index];
    }

    inline int indexOfSorted(const PitchColorMapEntry* const entry) const noexcept
    {
        jassert(this->collection[this->collection.indexOfSorted(*entryComparator, entry)] == entry);
        collection.indexOfSorted(*entryComparator, entry);
    }

private:
    WeakReference<Project> project;

    //==============================================================================
    IdGenerator::Id id;
    String name;
    OwnedArray<PitchColorMapEntry> collection;
    std::unordered_set<String> usedNames;
    std::unordered_set<int> usedNotes;

    //==============================================================================
    friend struct PitchColorMapNameComparator;

    std::unique_ptr<PitchColorMapEntryComparator> entryComparator{ new PitchColorMapEntryValueComparator(true) };

    //==============================================================================
    JUCE_LEAK_DETECTOR(PitchColorMap);
    JUCE_DECLARE_WEAK_REFERENCEABLE(PitchColorMap);
};

struct PitchColorMapNameComparator
{
    static inline int compareElements(const PitchColorMap& first, const PitchColorMap& second) noexcept
    {
        return PitchColorMapNameComparator::compareElements(&first, &second);
    }

    static int compareElements(const PitchColorMap* const first, const PitchColorMap* const second) noexcept;
};
