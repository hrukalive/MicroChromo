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

#include "Note.h"

#include "MidiTrack.h"

//==============================================================================
Note::Note() noexcept : MidiEvent(nullptr, Type::Note, 0.f) {}

Note::Note(WeakReference<MidiTrack> owner, const Note& parametersToCopy) noexcept :
    MidiEvent(owner, parametersToCopy),
    key(parametersToCopy.key),
    length(parametersToCopy.length),
    velocity(parametersToCopy.velocity),
    pitchColor(parametersToCopy.pitchColor) {}

Note::Note(WeakReference<MidiTrack> owner, int keyVal, float beatVal,
    float lengthVal, float velocityVal, String pitchColorVal) noexcept :
    MidiEvent(owner, Type::Note, beatVal),
    key(keyVal),
    length(lengthVal),
    velocity(velocityVal),
    pitchColor(pitchColorVal) {}

#define MIN_LENGTH (1.f / TICKS_PER_BEAT)

Note Note::withKey(int newKey) const noexcept
{
    Note other(*this);
    other.key = jlimit(0, 127, newKey);
    return other;
}

Note Note::withDeltaKey(int deltaKey) const noexcept
{
    Note other(*this);
    other.key = jlimit(0, 127, other.key + deltaKey);
    return other;
}

Note Note::withBeat(float newBeat) const noexcept
{
    Note other(*this);
    other.beat = roundBeat(newBeat);
    return other;
}

Note Note::withDeltaBeat(float deltaPosition) const noexcept
{
    Note other(*this);
    other.beat = roundBeat(other.beat + deltaPosition);
    return other;
}

Note Note::withKeyBeat(int newKey, float newBeat) const noexcept
{
    Note other(*this);
    other.key = jlimit(0, 127, newKey);
    other.beat = roundBeat(newBeat);
    return other;
}

Note Note::withKeyLength(int newKey, float newLength) const noexcept
{
    Note other(*this);
    other.key = jlimit(0, 127, newKey);
    other.length = jmax(MIN_LENGTH, newLength);
    return other;
}

Note Note::withLength(float newLength) const noexcept
{
    Note other(*this);
    other.length = jmax(MIN_LENGTH, newLength);
    return other;
}

Note Note::withDeltaLength(float deltaLength) const noexcept
{
    Note other(*this);
    other.length = jmax(MIN_LENGTH, other.length + deltaLength);
    return other;
}

Note Note::withVelocity(float newVelocity) const noexcept
{
    Note other(*this);
    other.velocity = jlimit(0.f, 1.f, newVelocity);
    return other;
}

Note Note::withPitchColor(String pitchColor) const noexcept
{
    Note other(*this);
    other.pitchColor = pitchColor;
    return other;
}

//===----------------------------------------------------------------------===//
// Accessors
//===----------------------------------------------------------------------===//
int Note::getKey() const noexcept
{
    return this->key;
}

float Note::getLength() const noexcept
{
    return this->length;
}

float Note::getVelocity() const noexcept
{
    return this->velocity;
}
String Note::getPitchColor() const noexcept
{
    return this->pitchColor;
}

void Note::setKey(int newKey) noexcept
{
    this->key = newKey;
}

void Note::setBeat(float newBeat) noexcept
{
    this->beat = newBeat;
}

void Note::setLength(float newLength) noexcept
{
    this->length = newLength;
}

void Note::setVelocity(float newVelocity) noexcept
{
    this->velocity = newVelocity;
}

void Note::setPitchColor(String newPitchColor) noexcept
{
    this->pitchColor = newPitchColor;
}

//===----------------------------------------------------------------------===//
// Serializable
//===----------------------------------------------------------------------===//
ValueTree Note::serialize() const noexcept
{
    ValueTree tree(Serialization::Midi::note);
    tree.setProperty(Serialization::Midi::key, this->key, nullptr);
    tree.setProperty(Serialization::Midi::timestamp, int(this->beat * TICKS_PER_BEAT), nullptr);
    tree.setProperty(Serialization::Midi::length, int(this->length * TICKS_PER_BEAT), nullptr);
    tree.setProperty(Serialization::Midi::volume, int(this->velocity * VELOCITY_SAVE_ACCURACY), nullptr);
    tree.setProperty(Serialization::Midi::pitchColor, this->pitchColor, nullptr);
    return tree;
}

ValueTree Note::serializeWithId() const noexcept
{
    ValueTree tree(Serialization::Midi::note);
    tree.setProperty(Serialization::Midi::id, this->id, nullptr);
    tree.setProperty(Serialization::Midi::key, this->key, nullptr);
    tree.setProperty(Serialization::Midi::timestamp, int(this->beat * TICKS_PER_BEAT), nullptr);
    tree.setProperty(Serialization::Midi::length, int(this->length * TICKS_PER_BEAT), nullptr);
    tree.setProperty(Serialization::Midi::volume, int(this->velocity * VELOCITY_SAVE_ACCURACY), nullptr);
    tree.setProperty(Serialization::Midi::pitchColor, this->pitchColor, nullptr);
    return tree;
}

void Note::deserialize(const ValueTree& tree) noexcept
{
    this->reset();

    const auto root =
        tree.hasType(Serialization::Midi::note) ?
        tree : tree.getChildWithName(Serialization::Midi::note);

    if (!root.isValid())
        return;

    if (tree.hasProperty(Serialization::Midi::id))
        this->id = tree.getProperty(Serialization::Midi::id, IdGenerator::generateId());
    this->key = tree.getProperty(Serialization::Midi::key);
    this->beat = float(tree.getProperty(Serialization::Midi::timestamp, 0)) / TICKS_PER_BEAT;
    this->length = float(tree.getProperty(Serialization::Midi::length, TICKS_PER_BEAT)) / TICKS_PER_BEAT;
    const auto vol = float(tree.getProperty(Serialization::Midi::volume, 0.8f)) / VELOCITY_SAVE_ACCURACY;
    this->velocity = jmax(jmin(vol, 1.f), 0.f);
    this->pitchColor = tree.getProperty(Serialization::Midi::pitchColor, "0");
}

void Note::reset() noexcept {}

//===------------------------------------------------------------------===//
// Helpers
//===------------------------------------------------------------------===//
void Note::applyChanges(const Note& other) noexcept
{
    jassert(this->id == other.id);
    this->key = other.key;
    this->beat = roundBeat(other.beat);
    this->length = other.length;
    this->velocity = other.velocity;
    this->pitchColor = other.pitchColor;
}

bool Note::equalWithoutId(const Note* const first, const Note* const second) noexcept
{
    return first->key == second->key && first->beat == second->beat &&
        first->length == second->length && first->velocity == second->velocity &&
        first->pitchColor == second->pitchColor;
}

int Note::compareElements(const Note* const first, const Note* const second) noexcept
{
    if (first == second) { return 0; }

    const float beatDiff = first->beat - second->beat;
    const int beatResult = (beatDiff > 0.f) - (beatDiff < 0.f);
    if (beatResult != 0) { return beatResult; }

    const int keyDiff = first->key - second->key;
    const int keyResult = (keyDiff > 0) - (keyDiff < 0);
    if (keyResult != 0) { return keyResult; }

    const int colorResult = first->pitchColor.compare(second->pitchColor);
    if (colorResult != 0) { return colorResult; }

    const int lenDiff = first->length - second->length;
    const int lenResult = (lenDiff > 0) - (lenDiff < 0);
    if (lenResult != 0) { return lenResult; }

    const int velDiff = first->velocity - second->velocity;
    const int velResult = (velDiff > 0) - (velDiff < 0);
    if (velResult != 0) { return velResult; }

    const int idDiff = first->id - second->id;
    const int idResult = (idDiff > 0) - (idDiff < 0);
    return idResult;
}
