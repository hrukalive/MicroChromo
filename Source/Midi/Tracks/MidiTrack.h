/*
  ==============================================================================

    MidiTrack.h
    Created: 12 Apr 2020 8:22:03pm
    Author:  bowen

  ==============================================================================
*/

#pragma once

#include "Common.h"
#include "Serializable.h"
#include "SerializationKeys.h"
#include "MidiEvent.h"

class Project;

class MidiTrack : public Serializable
{
public:
    explicit MidiTrack(Project& project) noexcept;

    //===------------------------------------------------------------------===//
    // Track editing
    //===------------------------------------------------------------------===//
    template<typename T>
    void importMidiEvent(const MidiEvent& eventToImport)
    {
        const auto& event = static_cast<const T&>(eventToImport);

        static T comparator;
        this->midiEvents.addSorted(comparator, new T(this, event));
    }

    //===------------------------------------------------------------------===//
    // Import/export
    //===------------------------------------------------------------------===//
    static float midiTicksToBeats(double ticks, int timeFormat) noexcept;
    virtual void importMidi(const MidiMessageSequence& sequence, short timeFormat) = 0;
    virtual void exportMidi(MidiMessageSequence& outSequence, double timeAdjustment, double timeFactor) const;

    //===------------------------------------------------------------------===//
    // Accessors
    //===------------------------------------------------------------------===//
    IdGenerator::Id getTrackId() const noexcept;

    virtual float getFirstBeat() const noexcept;
    virtual float getLastBeat() const noexcept;

    virtual int getTrackChannel() const noexcept;
    virtual void setTrackChannel(int val, bool sendNotifications, bool undoable);

    virtual String getTrackName() const noexcept;
    virtual void setTrackName(const String& val, bool sendNotifications, bool undoable);

    //===------------------------------------------------------------------===//
    // OwnedArray wrapper
    //===------------------------------------------------------------------===//
    void sort();

    inline int size() const noexcept
    {
        return this->midiEvents.size();
    }

    inline MidiEvent* const* begin() const noexcept
    {
        return this->midiEvents.begin();
    }

    inline MidiEvent* const* end() const noexcept
    {
        return this->midiEvents.end();
    }

    template<typename T = MidiEvent>
    inline T* getUnchecked(const int index) const noexcept
    {
        return dynamic_cast<T*>(this->midiEvents.getUnchecked(index));
    }

    inline int indexOfSorted(const MidiEvent* const event) const noexcept
    {
        jassert(this->midiEvents[this->midiEvents.indexOfSorted(*event, event)] == event);
        midiEvents.indexOfSorted(*event, event);
    }

    //===------------------------------------------------------------------===//
    // Helpers
    //===------------------------------------------------------------------===//
    void updateBeatRange(bool shouldNotifyIfChanged);
    void reset();

    static int compareElements(const MidiTrack* const first, const MidiTrack* const second) noexcept;

protected:
    friend class Project;

    void setTrackId(IdGenerator::Id _id) { id = _id; }

    Project& project;

    IdGenerator::Id id;
    String name;
    int channel = 1;
    OwnedArray<MidiEvent> midiEvents;

    float lastEndBeat;
    float lastStartBeat;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiTrack);
    JUCE_DECLARE_WEAK_REFERENCEABLE(MidiTrack);
};
