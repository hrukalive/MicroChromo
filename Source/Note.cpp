/*
    This file is part of Helio Workstation.

    Helio is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Helio is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Helio. If not, see <http://www.gnu.org/licenses/>.
*/

#include <JuceHeader.h>
#include "Note.h"

Note::Note() noexcept
{
    id = nidCounter++;
}

Note::Note(int keyVal, float beatVal,
    float lengthVal, float velocityVal, String pitchColorVal) noexcept :
    key(keyVal),
    beat(beatVal),
    length(lengthVal),
    velocity(velocityVal),
    pitchColor(pitchColorVal)
{
    id = nidCounter++;
}

#define TICKS_PER_BEAT 410
#define MIN_LENGTH (1.f / TICKS_PER_BEAT)

Note Note::withKey(int newKey) const noexcept
{
    Note other(*this);
    other.key = jlimit(0, 128, newKey);
    return other;
}

Note Note::withBeat(float newBeat) const noexcept
{
    Note other(*this);
    other.beat = newBeat;
    return other;
}

Note Note::withKeyBeat(int newKey, float newBeat) const noexcept
{
    Note other(*this);
    other.key = jlimit(0, 128, newKey);
    other.beat = newBeat;
    return other;
}

Note Note::withKeyLength(int newKey, float newLength) const noexcept
{
    Note other(*this);
    other.key = jlimit(0, 128, newKey);
    other.length = jmax(MIN_LENGTH, newLength);
    return other;
}

Note Note::withDeltaKey(int deltaKey) const noexcept
{
    Note other(*this);
    other.key = jlimit(0, 128, other.key + deltaKey);
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

uint32 Note::getId() const noexcept
{
    return this->id;
}

int Note::getKey() const noexcept
{
    return this->key;
}

float Note::getBeat() const noexcept
{
    return this->beat;
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

void Note::applyChanges(const Note& other) noexcept
{
    this->key = other.key;
    this->beat = other.beat;
    this->length = other.length;
    this->velocity = other.velocity;
    this->pitchColor = other.pitchColor;
}