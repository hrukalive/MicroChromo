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

#include "VoiceScheduler.h"

#include "Project.h"

//__________________________________________________________________________
//                                                                          |\
// InternalMidiMessage                                                      | |
//__________________________________________________________________________| |
//___________________________________________________________________________\|

VoiceScheduler::InternalMidiMessage::InternalMidiMessage() noexcept
{
    noteOn = true;
    noteMsg = MidiMessage::noteOn(1, 60, 0.8f).withTimeStamp(0);
    ccMsg = MidiMessage::controllerEvent(1, 102, 50).withTimeStamp(0);
}

VoiceScheduler::InternalMidiMessage::InternalMidiMessage(int channel, int key, 
    float timestamp, float velocity, int pitchbend, int cc, bool isNoteOn, 
    float adjustment, float pbRange, InternalMidiMessage* noteOnEvt)
{
    if (isNoteOn)
    {
        noteMsg = MidiMessage::noteOn(channel, key, velocity).withTimeStamp(timestamp);
        if (pbRange > 0)
            ccMsg = MidiMessage::pitchWheel(channel, 
                jlimit(0, 16383, (int)roundf(((pitchbend / 100.0f + pbRange) / (pbRange * 2)) * 16383)))
                    .withTimeStamp(jmax(0.0f, timestamp - adjustment));
        else
            ccMsg = MidiMessage::controllerEvent(channel, cc, pitchbend + 50).withTimeStamp(jmax(0.0f, timestamp - adjustment));
    }
    else
    {
        noteMsg = MidiMessage::noteOff(channel, key, velocity).withTimeStamp(timestamp);
    }
    noteOn = isNoteOn;
    noteOnPtr = noteOnEvt;
}

//===------------------------------------------------------------------===//
// Helpers
//===------------------------------------------------------------------===//
String VoiceScheduler::InternalMidiMessage::toString() const
{
    if (ccMsg.isController())
        return (noteOn ? " ON " : " OFF ") + ("key: " + String(noteMsg.getNoteNumber())) +
            " time: " + String(noteMsg.getTimeStamp(), 2) +
            " vel: " + String(noteMsg.getVelocity()) +
            " chn: " + String(noteMsg.getChannel()) + 
            (noteOn ? (" pit: " + String(ccMsg.getControllerValue() - 50) + " cc: " + String(ccMsg.getControllerNumber())) : "");
    else
        return (noteOn ? " ON " : " OFF ") + ("key: " + String(noteMsg.getNoteNumber())) +
            " time: " + String(noteMsg.getTimeStamp(), 2) +
            " vel: " + String(noteMsg.getVelocity()) +
            " chn: " + String(noteMsg.getChannel()) +
            (noteOn ? (" pb: " + String(ccMsg.getPitchWheelValue()) + " (" + 
                String((ccMsg.getPitchWheelValue() / 16383.0f) * 2 - 1, 4) + ")") : "");
}

int VoiceScheduler::InternalMidiMessage::compareElements(const InternalMidiMessage& first, const InternalMidiMessage& second)
{
    const float timeDiff = first.noteMsg.getTimeStamp() - second.noteMsg.getTimeStamp();
    const float timeResult = (timeDiff > 0.f) - (timeDiff < 0.f);
    if (timeResult != 0) return timeResult;

    const float keyDiff = first.noteMsg.getNoteNumber() - second.noteMsg.getNoteNumber();
    const float keyResult = (keyDiff > 0.f) - (keyDiff < 0.f);
    if (keyResult != 0) return keyResult;

    const float velDiff = second.noteMsg.getFloatVelocity() - first.noteMsg.getFloatVelocity();
    const float velResult = (velDiff > 0.f) - (velDiff < 0.f);
    if (velResult != 0) return velResult;

    return 0;
}

//__________________________________________________________________________
//                                                                          |\
// VoiceScheduler                                                           | |
//__________________________________________________________________________| |
//___________________________________________________________________________\|

VoiceScheduler::VoiceScheduler(Project& owner) : project(owner) {}

//===------------------------------------------------------------------===//
// Utility
//===------------------------------------------------------------------===//
void VoiceScheduler::schedule(OwnedArray<MidiMessageSequence>& noteSequences,
    OwnedArray<MidiMessageSequence>& ccSequences, int n, int ccBase, int modSource, 
    float pbRange, float ccTimeAdjustment, float timeMult)
{
    auto* colorMap = project.getPitchColorMap();
    auto* tempoTrack = project.getTempoTrack();
    for (int c = 1; c <= 16; c++)
    {
        OwnedArray<InternalMidiMessage> sequence;
        for (auto* track : project.getNoteTracks())
        {
            if (track->getTrackChannel() == c && track->size() > 0)
            {
                int j = 0;
                float timeFactor = 60 / 120.0f;
                float tempoElapsed = 0.0;
                float lastMarkerBeat = 0.0;
                for (int i = 0; i < track->size(); i++)
                {
                    auto* note = (*track)[i];
                    if (tempoTrack->size() > j && note->getBeat() >= (*tempoTrack)[j]->getBeat())
                    {
                        auto* marker = (*tempoTrack)[j];
                        tempoElapsed += timeFactor * (marker->getBeat() - lastMarkerBeat);
                        timeFactor = 60.0f / marker->getBPM();
                        lastMarkerBeat = marker->getBeat();
                        DBG("New Marker: " << marker->getBeat() << " " << marker->getBPM() << " " << lastMarkerBeat << " " << tempoElapsed);
                        j++;
                    }

                    auto key = note->getKey();
                    int pitchbend = 0;
                    if (modSource != USE_NONE)
                    {
                        if (auto* color = colorMap->findEntryByName(note->getPitchColor()))
                            pitchbend = color->getValue() + (project.getTuning(note->getKey()) - project.getTuning(9) + project.centDiffOfA());
                        while (pitchbend > 50)
                        {
                            key++;
                            pitchbend -= 100;
                        }
                        while (pitchbend < -50)
                        {
                            key--;
                            pitchbend += 100;
                        }
                        key = jlimit(0, 127, key);
                    }
                    auto beat = tempoElapsed + timeFactor * (note->getBeat() - lastMarkerBeat);
                    if (timeMult > 0)
                    {
                        beat = timeMult * note->getBeat();
                        timeFactor = timeMult;
                    }
                    InternalMidiMessage* onMsg;
                    if (modSource == USE_PITCHBEND)
                        onMsg = new InternalMidiMessage(c, key, beat, note->getVelocity(), pitchbend, ccBase, true, ccTimeAdjustment, pbRange);
                    else if (modSource == USE_KONTAKT)
                        onMsg = new InternalMidiMessage(c, key, beat, note->getVelocity(), pitchbend, ccBase + key % 12, true, ccTimeAdjustment, -1);
                    else
                        onMsg = new InternalMidiMessage(c, key, beat, note->getVelocity(), pitchbend, ccBase, true, ccTimeAdjustment, -1);
                    sequence.add(onMsg);
                    sequence.add(new InternalMidiMessage(c, key, beat + timeFactor * note->getLength(), note->getVelocity(), 0, -1, false, 0, -1, onMsg));
                }
            }
        }
        if (sequence.size() > 0)
        {
            sequence.sort(*sequence.getFirst(), true);
            if (modSource == USE_KONTAKT)
                scheduleKontakt(sequence, noteSequences, ccSequences, n);
            else
                scheduleGeneral(sequence, noteSequences, ccSequences, n);
        }
    }
    for (auto& seq : noteSequences)
    {
        seq->sort();
        seq->updateMatchedPairs();
    }
    for (auto& seq : ccSequences)
    {
        seq->sort();
        seq->updateMatchedPairs();
    }
}

void VoiceScheduler::scheduleGeneral(const OwnedArray<InternalMidiMessage>& sequence,
    OwnedArray<MidiMessageSequence>& noteSequences, OwnedArray<MidiMessageSequence>& ccSequences, int n)
{
    std::unordered_map<int, std::unordered_set<const InternalMidiMessage*>> channelToNote;
    std::unordered_map<int, int> channelToCcValue;
    std::unordered_map<const InternalMidiMessage*, int> noteToChannel;

    int channelUseCounter = 0;
    std::map<int, int> channelUseCount;
    for (int i = 0; i < n; i++)
    {
        channelToNote[i] = std::unordered_set<const InternalMidiMessage*>();
        channelToCcValue[i] = -2;
        channelUseCount[i] = 0;
    }

    for (auto* noteptr : sequence)
    {
        auto& note = *noteptr;
        bool needFindEmpty = true;
        bool needOverride = true;
        if (note.noteMsg.isNoteOn())
        {
            for (int i = 0; i < n; i++)
            {
                if ((note.ccMsg.isPitchWheel() && note.ccMsg.getPitchWheelValue() == channelToCcValue[i]) ||
                    (note.ccMsg.isController() && note.ccMsg.getControllerValue() == channelToCcValue[i]))
                {
                    noteSequences[i]->addEvent(note.noteMsg);

                    channelToNote[i].insert(&note);
                    noteToChannel[&note] = i;

                    channelUseCount[i] = ++channelUseCounter;

                    needFindEmpty = false;
                    needOverride = false;

                    DBG("Same v   " << i << " " << note.toString());
                    break;
                }
            }
            if (needFindEmpty)
            {
                int minIdx = -1, minVal = INT_MAX;
                for (auto& v : channelUseCount)
                {
                    if (v.second < minVal && channelToNote[v.first].empty())
                    {
                        minIdx = v.first;
                        minVal = v.second;
                    }
                }
                if (minIdx != -1)
                {
                    noteSequences[minIdx]->addEvent(note.noteMsg);
                    ccSequences[minIdx]->addEvent(note.ccMsg);

                    channelToNote[minIdx].insert(&note);
                    channelToCcValue[minIdx] = note.ccMsg.isController() ? note.ccMsg.getControllerValue() : note.ccMsg.getPitchWheelValue();
                    noteToChannel[&note] = minIdx;

                    channelUseCount[minIdx] = ++channelUseCounter;

                    needOverride = false;

                    DBG("Add fv   " << minIdx << " " << note.toString());
                    DBG("Add cc   " << minIdx << " " << note.toString());
                }
            }
            if (needOverride)
            {
                int minIdx = -1, minVal = INT_MAX;
                for (auto& v : channelUseCount)
                {
                    if (v.second < minVal)
                    {
                        minIdx = v.first;
                        minVal = v.second;
                    }
                }

                auto& allNotesNow = channelToNote[minIdx];
                for (auto noteNum : allNotesNow)
                {
                    MidiMessage msg = MidiMessage::noteOff(note.noteMsg.getChannel(), 
                        noteNum->noteMsg.getNoteNumber()).withTimeStamp(note.noteMsg.getTimeStamp());
                    noteSequences[minIdx]->addEvent(msg);
                    noteToChannel.erase(noteNum);
                }
                channelToNote[minIdx].clear();

                noteSequences[minIdx]->addEvent(note.noteMsg);
                ccSequences[minIdx]->addEvent(note.ccMsg);

                channelToNote[minIdx].insert(&note);
                channelToCcValue[minIdx] = note.ccMsg.isController() ? note.ccMsg.getControllerValue() : note.ccMsg.getPitchWheelValue();
                noteToChannel[&note] = minIdx;

                channelUseCount[minIdx] = ++channelUseCounter;

                DBG("Override " << minIdx << " " << note.toString());
            }
        }
        else
        {
            if (note.noteOnPtr != nullptr && noteToChannel.find(note.noteOnPtr) != noteToChannel.end())
            {
                auto channel = noteToChannel[note.noteOnPtr];
                noteSequences[channel]->addEvent(note.noteMsg);
                noteToChannel.erase(note.noteOnPtr);
                channelToNote[channel].erase(note.noteOnPtr);

                DBG("Note off " << channel << " " << note.toString());
            }
        }
    }
}

void VoiceScheduler::scheduleKontakt(const OwnedArray<InternalMidiMessage>& sequence,
    OwnedArray<MidiMessageSequence>& noteSequences, OwnedArray<MidiMessageSequence>& ccSequences, int n)
{
    std::unordered_map<int, std::unordered_set<const InternalMidiMessage*>> channelToNote;
    std::unordered_map<int, int> channelToCcValue;
    std::unordered_map<const InternalMidiMessage*, int> noteToChannel;

    int channelUseCounter = 0;
    std::map<int, int> channelUseCount;
    for (int i = 0; i < n * 12; i++)
    {
        channelToNote[i] = std::unordered_set<const InternalMidiMessage*>();
        channelToCcValue[i] = -2;
        channelUseCount[i] = 0;
    }

    for (auto* noteptr : sequence)
    {
        auto& note = *noteptr;
        bool needFindEmpty = true;
        bool needOverride = true;
        if (note.noteMsg.isNoteOn())
        {
            for (int i = 0; i < n; i++)
            {
                auto realIdx = i * 12 + note.noteMsg.getNoteNumber() % 12;
                if (note.ccMsg.getControllerValue() == channelToCcValue[realIdx])
                {
                    noteSequences[i]->addEvent(note.noteMsg);

                    channelToNote[realIdx].insert(&note);
                    noteToChannel[&note] = realIdx;

                    channelUseCount[realIdx] = ++channelUseCounter;

                    needFindEmpty = false;
                    needOverride = false;

                    DBG("Same v   " << i << " " << realIdx % 12 << note.toString());
                    break;
                }
            }
            if (needFindEmpty)
            {
                int minIdx = -1, minVal = INT_MAX;
                for (int i = note.noteMsg.getNoteNumber() % 12; i < n * 12; i += 12)
                {
                    auto v = channelUseCount[i];
                    if (v < minVal && channelToNote[i].empty())
                    {
                        minIdx = i;
                        minVal = v;
                    }
                }
                int seqIndex = minIdx / 12;

                if (minIdx != -1)
                {
                    noteSequences[seqIndex]->addEvent(note.noteMsg);
                    ccSequences[seqIndex]->addEvent(note.ccMsg);

                    channelToNote[minIdx].insert(&note);
                    channelToCcValue[minIdx] = note.ccMsg.getControllerValue();
                    noteToChannel[&note] = minIdx;

                    channelUseCount[minIdx] = ++channelUseCounter;

                    needOverride = false;

                    DBG("Add fv   " << seqIndex << " " << minIdx % 12 << note.toString());
                    DBG("Add cc   " << seqIndex << " " << minIdx % 12 << note.toString());
                }
            }
            if (needOverride)
            {
                int minIdx = -1, minVal = INT_MAX;
                for (int i = note.noteMsg.getNoteNumber() % 12; i < n * 12; i += 12)
                {
                    auto v = channelUseCount[i];
                    if (v < minVal)
                    {
                        minIdx = i;
                        minVal = v;
                    }
                }
                int seqIndex = minIdx / 12;

                auto& allNotesNow = channelToNote[minIdx];
                for (auto* noteNum : allNotesNow)
                {
                    MidiMessage msg = MidiMessage::noteOff(note.noteMsg.getChannel(), 
                        noteNum->noteMsg.getNoteNumber()).withTimeStamp(note.noteMsg.getTimeStamp());
                    noteSequences[seqIndex]->addEvent(msg);
                    noteToChannel.erase(noteNum);
                }
                channelToNote[minIdx].clear();

                noteSequences[seqIndex]->addEvent(note.noteMsg);
                ccSequences[seqIndex]->addEvent(note.ccMsg);

                channelToNote[minIdx].insert(&note);
                channelToCcValue[minIdx] = note.ccMsg.getControllerValue();
                noteToChannel[&note] = minIdx;

                channelUseCount[minIdx] = ++channelUseCounter;

                DBG("Override " << seqIndex << " " << minIdx % 12 << note.toString());
            }
        }
        else
        {
            if (note.noteOnPtr != nullptr && noteToChannel.find(note.noteOnPtr) != noteToChannel.end())
            {
                auto channel = noteToChannel[note.noteOnPtr];
                noteSequences[channel / 12]->addEvent(note.noteMsg);
                noteToChannel.erase(note.noteOnPtr);
                channelToNote[channel].erase(note.noteOnPtr);

                DBG("Note off " << channel / 12 << " " << channel % 12 << note.toString());
            }
        }
    }
}
