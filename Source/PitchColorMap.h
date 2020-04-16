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

    PitchColorMapEntry(const PitchColorMapEntry& other) noexcept = default;
    PitchColorMapEntry& operator= (const PitchColorMapEntry& other) = default;

    PitchColorMapEntry(PitchColorMapEntry&& other) noexcept = default;
    PitchColorMapEntry& operator= (PitchColorMapEntry&& other) noexcept = default;

    PitchColorMapEntry(WeakReference<PitchColorMap> owner, 
        const PitchColorMapEntry& parametersToCopy) noexcept;
    explicit PitchColorMapEntry(WeakReference<PitchColorMap> owner, String name = "0", 
        int value = 0, Colour color = Colours::grey, int defaultKey = -1) noexcept;

    PitchColorMapEntry withName(const String newName);
    PitchColorMapEntry withValue(const int newValue);
    PitchColorMapEntry withColor(const Colour& newColor);
    PitchColorMapEntry withDefaultKey(const int newkey);

    //===------------------------------------------------------------------===//
    // Accessors
    //===------------------------------------------------------------------===//
    IdGenerator::Id getEntryId() const noexcept;
    String getName() const noexcept;
    int getValue() const noexcept;
    Colour getColor() const noexcept;
    int getDefaultKey() const noexcept;
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

    static inline int compareElements(const PitchColorMapEntry& first, const PitchColorMapEntry& second) noexcept
    {
        return PitchColorMapEntry::compareElements(&first, &second);
    }

    static int compareElements(const PitchColorMapEntry* const first, const PitchColorMapEntry* const second) noexcept;

private:
    WeakReference<PitchColorMap> colorMap;
    IdGenerator::Id id;
    String name;
    int value;
    Colour color;
    int defaultKey;

    JUCE_LEAK_DETECTOR(PitchColorMapEntry)
};

class PitchColorMap : public Serializable
{
public:
    PitchColorMap(Project& project) noexcept;
    PitchColorMap(Project& project, String name, const Array<PitchColorMapEntry>& entries) noexcept;

    //===------------------------------------------------------------------===//
    // Accessors
    //===------------------------------------------------------------------===//
    PitchColorMapEntry* getEntryById(IdGenerator::Id entryId);
    Array<PitchColorMapEntry> getAllEntries();

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

    inline int indexOfSorted(const PitchColorMapEntry* const entry) const noexcept
    {
        jassert(this->collection[this->collection.indexOfSorted(*entry, entry)] == entry);
        collection.indexOfSorted(*entry, entry);
    }

private:
    Project& project;
    String name;
    OwnedArray<PitchColorMapEntry> collection;
    std::unordered_set<String> usedNames;

    friend struct PitchColorMapNameComparator;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PitchColorMap);
    JUCE_DECLARE_WEAK_REFERENCEABLE(PitchColorMap);
};

struct PitchColorMapNameComparator
{
    static int compareElements(const PitchColorMap& first, const PitchColorMap& second);
};
