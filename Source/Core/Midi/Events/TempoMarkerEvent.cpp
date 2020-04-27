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

#include "TempoMarkerEvent.h"

#include "MidiTrack.h"

//==============================================================================
TempoMarkerEvent::TempoMarkerEvent() noexcept : MidiEvent(nullptr, Type::TempoMarker, 0.f) {}

TempoMarkerEvent::TempoMarkerEvent(WeakReference<MidiTrack> owner, const TempoMarkerEvent& parametersToCopy) noexcept :
    MidiEvent(owner, parametersToCopy),
    bpm(parametersToCopy.bpm) {}

TempoMarkerEvent::TempoMarkerEvent(WeakReference<MidiTrack> owner, float beatVal, float bpmVal) noexcept :
    MidiEvent(owner, Type::TempoMarker, beatVal),
    bpm(bpmVal) {}

TempoMarkerEvent TempoMarkerEvent::withDeltaBeat(float beatOffset) const noexcept
{
    TempoMarkerEvent e(*this);
    e.beat = e.beat + beatOffset;
    return e;
}

TempoMarkerEvent TempoMarkerEvent::withBeat(float newBeat) const noexcept
{
    TempoMarkerEvent e(*this);
    e.beat = newBeat;
    return e;
}

TempoMarkerEvent TempoMarkerEvent::withBPM(const float newBpm) const noexcept
{
    TempoMarkerEvent e(*this);
    e.bpm = jlimit(1.0f, 999.0f, newBpm);
    return e;
}

//===------------------------------------------------------------------===//
// Accessors
//===------------------------------------------------------------------===//
float TempoMarkerEvent::getBPM() const noexcept
{
    return bpm;
}

//===----------------------------------------------------------------------===//
// Serializable
//===----------------------------------------------------------------------===//
ValueTree TempoMarkerEvent::serialize() const noexcept
{
    ValueTree tree(Serialization::Midi::tempoMarker);
    tree.setProperty(Serialization::Midi::timestamp, int(this->beat * TICKS_PER_BEAT), nullptr);
    tree.setProperty(Serialization::Midi::bpm, this->bpm, nullptr);
    return tree;
}

void TempoMarkerEvent::deserialize(const ValueTree& tree) noexcept
{
    this->reset();

    const auto root =
        tree.hasType(Serialization::Midi::tempoMarker) ?
        tree : tree.getChildWithName(Serialization::Midi::tempoMarker);

    if (!root.isValid())
        return;

    this->beat = float(tree.getProperty(Serialization::Midi::timestamp, 0)) / TICKS_PER_BEAT;
    this->bpm = tree.getProperty(Serialization::Midi::bpm, TEMPO_DEFAULT_BPM);
    this->bpm = jlimit(1.0f, 999.0f, this->bpm);
}

void TempoMarkerEvent::reset() noexcept {}

//===------------------------------------------------------------------===//
// Helpers
//===------------------------------------------------------------------===//
void TempoMarkerEvent::applyChanges(const TempoMarkerEvent & parameters) noexcept
{
    jassert(this->id == parameters.id);
    this->beat = parameters.beat;
    this->bpm = jlimit(1.0f, 999.0f, parameters.bpm);
}

bool TempoMarkerEvent::equalWithoutId(const TempoMarkerEvent* const first, const 
    TempoMarkerEvent* const second) noexcept
{
    return first->beat == second->beat && first->bpm == second->bpm;
}
