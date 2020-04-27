/*
  ==============================================================================

    PitchColorMap.h
    Created: 14 Apr 2020 7:04:35pm
    Author:  bowen

  ==============================================================================
*/

#pragma once

#include "Common.h"
#include "Serializable.h"
class Project;
class PitchColorMap;

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
        int value, Colour color, const std::unordered_set<int>& defaultSet) noexcept;
    explicit PitchColorMapEntry(WeakReference<PitchColorMap> owner, String name = "0", 
        int value = 0, Colour color = Colours::grey) noexcept;

    PitchColorMapEntry withName(const String newName);
    PitchColorMapEntry withValue(const int newValue);
    PitchColorMapEntry withColor(const Colour& newColor);
    PitchColorMapEntry withDefaultKeys(const std::unordered_set<int>& newkeys);

    //===------------------------------------------------------------------===//
    // Accessors
    //===------------------------------------------------------------------===//
    IdGenerator::Id getEntryId() const noexcept;
    String getName() const noexcept;
    int getValue() const noexcept;
    Colour getColor() const noexcept;
    std::unordered_set<int> getDefaultKeys() const noexcept;
    PitchColorMap* getPitchColorMap();

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

    static inline bool equalWithoutId(const PitchColorMapEntry& first, const PitchColorMapEntry& second) noexcept
    {
        return PitchColorMapEntry::equalWithoutId(&first, &second);
    }

    static bool equalWithoutId(const PitchColorMapEntry* const first, const PitchColorMapEntry* const second) noexcept;

    static inline int compareElements(const PitchColorMapEntry& first, const PitchColorMapEntry& second) noexcept
    {
        return PitchColorMapEntry::compareElements(&first, &second);
    }

    static int compareElements(const PitchColorMapEntry* const first, const PitchColorMapEntry* const second) noexcept;

private:
    //===------------------------------------------------------------------===//
    // Helpers
    //===------------------------------------------------------------------===//
    String defaultKeysToString() const noexcept;
    std::unordered_set<int> stringToDefaultKeys(const String& str) const noexcept;

    WeakReference<PitchColorMap> colorMap;
    IdGenerator::Id id;
    String name;
    int value;
    Colour color;
    std::unordered_set<int> defaultKeys;

    JUCE_LEAK_DETECTOR(PitchColorMapEntry)
};

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
        jassert(this->collection[this->collection.indexOfSorted(*entry, entry)] == entry);
        collection.indexOfSorted(*entry, entry);
    }

private:
    WeakReference<Project> project;

    IdGenerator::Id id;
    String name;
    OwnedArray<PitchColorMapEntry> collection;
    std::unordered_set<String> usedNames;
    std::unordered_set<int> usedNotes;

    friend struct PitchColorMapNameComparator;

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
