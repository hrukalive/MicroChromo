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

#include "MidiEvent.h"

//==============================================================================
MidiEvent::MidiEvent(WeakReference<MidiTrack> owner, const MidiEvent& parameters) noexcept :
    track(owner),
    type(parameters.type),
    id(parameters.id)
{
    beat = roundBeat(parameters.beat);
}

MidiEvent::MidiEvent(WeakReference<MidiTrack> owner, Type type, float beatVal) noexcept
    : track(owner), type(type)
{
    id = IdGenerator::generateId();
    beat = roundBeat(beatVal);
}

//===------------------------------------------------------------------===//
// Accessors
//===------------------------------------------------------------------===//
bool MidiEvent::isValid() const noexcept
{
    return this->track != nullptr;
}

MidiTrack* MidiEvent::getTrack() const noexcept
{
    jassert(track);
    return track;
}

IdGenerator::Id MidiEvent::getId() const noexcept
{
    return id;
}

float MidiEvent::getBeat() const noexcept
{
    return beat;
}

//===------------------------------------------------------------------===//
// Helpers
//===------------------------------------------------------------------===//
bool MidiEvent::equalWithoutId(const MidiEvent* const first, const MidiEvent* const second) noexcept
{
    return first->beat == second->beat;
}

int MidiEvent::compareElements(const MidiEvent* const first, const MidiEvent* const second) noexcept
{
    if (first == second) { return 0; }

    const float beatDiff = first->beat - second->beat;
    const int beatResult = (beatDiff > 0.f) - (beatDiff < 0.f);
    if (beatResult != 0) { return beatResult; }

    const int idDiff = first->id - second->id;
    const int idResult = (idDiff > 0) - (idDiff < 0);
    return idResult;
}
