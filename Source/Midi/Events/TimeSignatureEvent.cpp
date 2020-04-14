/*
  ==============================================================================

    TimeSignatureEvent.cpp
    Created: 12 Apr 2020 8:23:29pm
    Author:  bowen

  ==============================================================================
*/

#include "Common.h"
#include "TimeSignatureEvent.h"
#include "MidiTrack.h"

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

void TimeSignatureEvent::exportMessages(MidiMessageSequence& outSequence, double timeOffset, double timeFactor) const noexcept
{
    outSequence.addEvent(MidiMessage::timeSignatureMetaEvent(numerator, denominator).withTimeStamp(beat * timeFactor + timeOffset));
}

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
    using namespace Serialization;
    ValueTree tree(Midi::timeSignature);
    tree.setProperty(Midi::id, this->id, nullptr);
    tree.setProperty(Midi::timestamp, int(this->beat * TICKS_PER_BEAT), nullptr);
    tree.setProperty(Midi::numerator, this->numerator, nullptr);
    tree.setProperty(Midi::denominator, this->denominator, nullptr);
    return tree;
}

void TimeSignatureEvent::deserialize(const ValueTree& tree) noexcept
{
    this->reset();
    using namespace Serialization;
    this->id = tree.getProperty(Midi::id);
    this->beat = float(tree.getProperty(Midi::timestamp)) / TICKS_PER_BEAT;
    this->numerator = tree.getProperty(Midi::numerator, TIME_SIGNATURE_DEFAULT_NUMERATOR);
    this->denominator = tree.getProperty(Midi::denominator, TIME_SIGNATURE_DEFAULT_DENOMINATOR);
}

void TimeSignatureEvent::reset() noexcept {}

void TimeSignatureEvent::applyChanges(const TimeSignatureEvent& parameters) noexcept
{
    jassert(this->id == parameters.id);
    this->beat = parameters.beat;
    this->numerator = parameters.numerator;
    this->denominator = parameters.denominator;
}
