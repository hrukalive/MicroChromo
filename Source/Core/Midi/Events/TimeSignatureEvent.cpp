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

#include "TimeSignatureEvent.h"

#include "MidiTrack.h"

//==============================================================================
TimeSignatureEvent::TimeSignatureEvent() noexcept : MidiEvent(nullptr, Type::TimeSignature, 0.f) {}

TimeSignatureEvent::TimeSignatureEvent(WeakReference<MidiTrack> owner, const TimeSignatureEvent& parametersToCopy) noexcept :
    MidiEvent(owner, parametersToCopy),
    numerator(parametersToCopy.numerator),
    denominator(parametersToCopy.denominator) {}

TimeSignatureEvent::TimeSignatureEvent(WeakReference<MidiTrack> owner, float beatVal,
    float numeratorVal, float denominatorVal) noexcept :
    MidiEvent(owner, Type::TimeSignature, beatVal),
    numerator(numeratorVal),
    denominator(denominatorVal) {}

TimeSignatureEvent TimeSignatureEvent::withDeltaBeat(float beatOffset) const noexcept
{
    TimeSignatureEvent e(*this);
    e.beat = e.beat + beatOffset;
    return e;
}

TimeSignatureEvent TimeSignatureEvent::withBeat(float newBeat) const noexcept
{
    TimeSignatureEvent e(*this);
    e.beat = newBeat;
    return e;
}

TimeSignatureEvent TimeSignatureEvent::withNumerator(const int newNumerator) const noexcept
{
    TimeSignatureEvent e(*this);
    e.numerator = newNumerator;
    return e;
}

TimeSignatureEvent TimeSignatureEvent::withDenominator(const int newDenominator) const noexcept
{
    TimeSignatureEvent e(*this);
    e.denominator = newDenominator;
    return e;
}

//===----------------------------------------------------------------------===//
// Accessors
//===----------------------------------------------------------------------===//
int TimeSignatureEvent::getNumerator() const noexcept
{
    return this->numerator;
}

int TimeSignatureEvent::getDenominator() const noexcept
{
    return this->denominator;
}

//===----------------------------------------------------------------------===//
// Serializable
//===----------------------------------------------------------------------===//
ValueTree TimeSignatureEvent::serialize() const noexcept
{
    ValueTree tree(Serialization::Midi::timeSignature);
    tree.setProperty(Serialization::Midi::timestamp, int(this->beat * TICKS_PER_BEAT), nullptr);
    tree.setProperty(Serialization::Midi::numerator, this->numerator, nullptr);
    tree.setProperty(Serialization::Midi::denominator, this->denominator, nullptr);
    return tree;
}

void TimeSignatureEvent::deserialize(const ValueTree& tree) noexcept
{
    this->reset();

    const auto root =
        tree.hasType(Serialization::Midi::timeSignature) ?
        tree : tree.getChildWithName(Serialization::Midi::timeSignature);

    if (!root.isValid())
        return;

    this->beat = float(tree.getProperty(Serialization::Midi::timestamp, 0)) / TICKS_PER_BEAT;
    this->numerator = tree.getProperty(Serialization::Midi::numerator, TIME_SIGNATURE_DEFAULT_NUMERATOR);
    this->denominator = tree.getProperty(Serialization::Midi::denominator, TIME_SIGNATURE_DEFAULT_DENOMINATOR);
}

void TimeSignatureEvent::reset() noexcept {}

//===------------------------------------------------------------------===//
// Helpers
//===------------------------------------------------------------------===//
void TimeSignatureEvent::applyChanges(const TimeSignatureEvent& parameters) noexcept
{
    jassert(this->id == parameters.id);
    this->beat = parameters.beat;
    this->numerator = parameters.numerator;
    this->denominator = parameters.denominator;
}

bool TimeSignatureEvent::equalWithoutId(const TimeSignatureEvent* const first, 
    const TimeSignatureEvent* const second) noexcept
{
    return first->beat == second->beat && first->numerator == second->numerator &&
        first->denominator == second->denominator;
}
