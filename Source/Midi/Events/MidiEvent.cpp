/*
  ==============================================================================

    MidiEvent.cpp
    Created: 12 Apr 2020 8:22:14pm
    Author:  bowen

  ==============================================================================
*/

#include "MidiEvent.h"

MidiEvent::MidiEvent(WeakReference<MidiTrack> owner, const MidiEvent& parameters) noexcept :
    track(owner),
    type(parameters.type),
    beat(parameters.beat),
    id(parameters.id) {}

MidiEvent::MidiEvent(WeakReference<MidiTrack> owner, Type type, float beatVal) noexcept
    : track(owner), type(type), beat(beatVal)
{
    id = IdGenerator::generateId();
}

bool MidiEvent::isValid() const noexcept
{
    return this->track != nullptr;
}

MidiTrack* MidiEvent::getTrack() const noexcept
{
    jassert(track);
    return track;
}

uint32 MidiEvent::getId() const noexcept
{
    return id;
}

float MidiEvent::getBeat() const noexcept
{
    return beat;
}

int MidiEvent::compareElements(const MidiEvent* const first, const MidiEvent* const second) noexcept
{
    const float timeDiff = first->getBeat() - second->getBeat();
    return (timeDiff > 0.f) - (timeDiff < 0.f);
}
