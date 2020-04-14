/*
  ==============================================================================

    MidiTrack.cpp
    Created: 12 Apr 2020 8:22:03pm
    Author:  bowen

  ==============================================================================
*/

#include "MidiTrack.h"
#include "Project.h"

MidiTrack::MidiTrack(Project& owner) noexcept :
    project(owner),
    name("New Track"),
    channel(1),
    lastStartBeat(0.f),
    lastEndBeat(0.f)
{
    id = IdGenerator::generateId();
}

template<typename T>
void MidiTrack::importMidiEvent(const MidiEvent& eventToImport)
{
    const auto& event = static_cast<const T&>(eventToImport);

    static T comparator;
    this->midiEvents.addSorted(comparator, new T(this, event));
}
//===----------------------------------------------------------------------===//
// Import/export
//===----------------------------------------------------------------------===//

void MidiTrack::exportMidi(MidiMessageSequence& outSequence, double timeAdjustment, double timeFactor) const
{
    for (const auto* event : this->midiEvents)
        event->exportMessages(outSequence, timeAdjustment, timeFactor);

    outSequence.updateMatchedPairs();
}

float MidiTrack::midiTicksToBeats(double ticks, int timeFormat) noexcept
{
    const double secsPerQuarterNoteAt120BPM = 0.5;

    if (timeFormat < 0)
    {
        const double timeInSeconds = ticks / (-(timeFormat >> 8) * (timeFormat & 0xff));
        return float(timeInSeconds * secsPerQuarterNoteAt120BPM);
    }

    const auto tickLen = 1.0 / (timeFormat & 0x7fff);
    const auto secsPerTick = 0.5 * tickLen;
    const double timeInSeconds = ticks * secsPerTick;
    return float(timeInSeconds * secsPerQuarterNoteAt120BPM);
}

//===----------------------------------------------------------------------===//
// Accessors
//===----------------------------------------------------------------------===//
float MidiTrack::getFirstBeat() const noexcept
{
    if (this->midiEvents.size() == 0)
    {
        return FLT_MAX;
    }

    return midiEvents.getFirst()->getBeat();
}

float MidiTrack::getLastBeat() const noexcept
{
    if (this->midiEvents.size() == 0)
    {
        return -FLT_MAX;
    }

    return midiEvents.getLast()->getBeat();
}

int MidiTrack::getTrackChannel() const noexcept
{
    return channel;
}
void MidiTrack::setTrackChannel(int val, bool sendNotifications)
{
    channel = val;
}

String MidiTrack::getTrackName() const noexcept
{
    return name;
}
void MidiTrack::setTrackName(const String& val, bool sendNotifications)
{
    name = val;
}

void MidiTrack::sort()
{
    if (this->midiEvents.size() > 0)
    {
        this->midiEvents.sort(*this->midiEvents.getFirst(), true);
    }
}

void MidiTrack::updateBeatRange(bool shouldNotifyIfChanged)
{
    lastStartBeat = getFirstBeat();
    lastEndBeat = getLastBeat();

    if (shouldNotifyIfChanged)
    {
        project.broadcastChangeProjectBeatRange();
    }
}

void MidiTrack::reset()
{
    midiEvents.clear();
}
