/*
  ==============================================================================

    MidiTrack.cpp
    Created: 12 Apr 2020 8:22:03pm
    Author:  bowen

  ==============================================================================
*/

#include "MidiTrack.h"
#include "Project.h"
#include "NoteTrackActions.h"

MidiTrack::MidiTrack(Project& owner) noexcept :
    project(owner),
    name("New Track"),
    channel(1),
    lastStartBeat(0.f),
    lastEndBeat(0.f)
{
    id = IdGenerator::generateId();
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
IdGenerator::Id MidiTrack::getTrackId() const noexcept
{
    return id;
}


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
void MidiTrack::setTrackChannel(int val, bool sendNotifications, bool undoable)
{
    if (channel == val)
        return;
    if (undoable)
    {
        project.getUndoManager().
            perform(new MidiTrackChannelChangeAction(project,
                getTrackId(), val));
    }
    else
    {
        channel = val;

        if (sendNotification)
            project.broadcastChangeTrackProperties(this);
    }
}

String MidiTrack::getTrackName() const noexcept
{
    return name;
}
void MidiTrack::setTrackName(const String& val, bool sendNotifications, bool undoable)
{
    if (name == val)
        return;
    if (undoable)
    {
        project.getUndoManager().
            perform(new MidiTrackRenameAction(project,
                getTrackId(), val));
    }
    else
    {
        name = val;

        if (sendNotification)
            project.broadcastChangeTrackProperties(this);
    }
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

int MidiTrack::compareElements(const MidiTrack* const first, const MidiTrack* const second) noexcept
{
    if (first == second)
        return 0;

    const float nameResult = first->name.compare(second->name);
    if (nameResult != 0) return nameResult;

    const float channelDiff = first->channel - second->channel;
    const int channelResult = (channelDiff > 0.f) - (channelDiff < 0.f);
    if (channelResult != 0) return channelResult;

    const float idDiff = first->id - second->id;
    return (idDiff > 0.f) - (idDiff < 0.f);
}
