/*
  ==============================================================================

    VoiceScheduler.cpp
    Created: 23 Apr 2020 7:58:17pm
    Author:  bowen

  ==============================================================================
*/

#include "VoiceScheduler.h"
#include "Project.h"

VoiceScheduler::InternalMidiMessage::InternalMidiMessage() noexcept
{
    noteOn = true;
    noteMsg = MidiMessage::noteOn(1, 60, 0.8f).withTimeStamp(0);
    ccMsg = MidiMessage::controllerEvent(1, 102, 50).withTimeStamp(0);
}

VoiceScheduler::InternalMidiMessage::InternalMidiMessage(int channel, int key, 
    float timestamp, float velocity, int pitchbend, int cc, bool isNoteOn, float adjustment)
{
    if (isNoteOn)
    {
        noteMsg = MidiMessage::noteOn(channel, key, velocity).withTimeStamp(timestamp);
        ccMsg = MidiMessage::controllerEvent(channel, cc, pitchbend + 50).withTimeStamp(jmax(0.0f, timestamp - adjustment));
    }
    else
    {
        noteMsg = MidiMessage::noteOff(channel, key, velocity).withTimeStamp(timestamp);
    }
    noteOn = isNoteOn;
}

String VoiceScheduler::InternalMidiMessage::toString() const
{
    return (noteOn ? " ON " : " OFF ") + ("key: " + String(noteMsg.getNoteNumber())) +
        " time: " + String(noteMsg.getTimeStamp(), 2) +
        " vel: " + String(noteMsg.getVelocity()) +
        " chn: " + String(noteMsg.getChannel()) + 
        (noteOn ? (" pit: " + String(ccMsg.getControllerValue() - 50) + " cc: " + String(ccMsg.getControllerNumber())) : "");
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

VoiceScheduler::VoiceScheduler(Project& owner) : project(owner) {}

void VoiceScheduler::schedule(OwnedArray<MidiMessageSequence>& noteSequences,
    OwnedArray<MidiMessageSequence>& ccSequences, int n, int ccBase, bool kontaktMode, 
    float ccTimeAdjustment, float timeMult)
{
    auto* colorMap = project.getPitchColorMap();
    auto* tempoTrack = project.getTempoTrack();
    for (int c = 1; c <= 16; c++)
    {
        for (auto* track : project.getNoteTracks())
        {
            if (track->getTrackChannel() == c && track->size() > 0)
            {
                int j = 0;
                float timeFactor = 60 / 120.0f;
                float tempoElapsed = 0.0;
                float lastMarkerBeat = 0.0;
                Array<InternalMidiMessage> sequence;
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

                    int pitchbend = 0;
                    if (auto* color = colorMap->findEntryByName(note->getPitchColor()))
                        pitchbend = color->getValue();
                    auto key = note->getKey();
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
                    auto beat = tempoElapsed + timeFactor * (note->getBeat() - lastMarkerBeat);
                    if (timeMult > 0)
                    {
                        beat = timeMult * note->getBeat();
                        timeFactor = timeMult;
                    }
                    if (kontaktMode)
                        sequence.add(InternalMidiMessage(c, key, beat, note->getVelocity(), pitchbend, ccBase + key % 12, true, ccTimeAdjustment));
                    else
                        sequence.add(InternalMidiMessage(c, key, beat, note->getVelocity(), pitchbend, ccBase, true, ccTimeAdjustment));
                    sequence.add(InternalMidiMessage(c, key, beat + timeFactor * note->getLength(), note->getVelocity(), 0, -1, false, 0));
                }
                sequence.sort(sequence.getFirst(), true);

                if (kontaktMode)
                    scheduleKontakt(sequence, noteSequences, ccSequences, n);
                else
                    scheduleGeneral(sequence, noteSequences, ccSequences, n);
            }
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

void VoiceScheduler::scheduleGeneral(const Array<InternalMidiMessage>& sequence,
    OwnedArray<MidiMessageSequence>& noteSequences, OwnedArray<MidiMessageSequence>& ccSequences, int n)
{
    std::unordered_map<int, std::unordered_set<int>> channelToNote;
    std::unordered_map<int, int> channelToCcValue;
    std::unordered_map<int, int> noteToChannel;

    int channelUseCounter = 0;
    std::map<int, int> channelUseCount;
    for (int i = 0; i < n; i++)
    {
        channelToNote[i] = std::unordered_set<int>();
        channelToCcValue[i] = -2;
        channelUseCount[i] = 0;
    }

    for (auto& note : sequence)
    {
        bool needFindEmpty = true;
        bool needOverride = true;
        if (note.noteMsg.isNoteOn())
        {
            for (int i = 0; i < n; i++)
            {
                if (note.ccMsg.getControllerValue() == channelToCcValue[i])
                {
                    noteSequences[i]->addEvent(note.noteMsg);

                    channelToNote[i].insert(note.noteMsg.getNoteNumber());
                    noteToChannel[note.noteMsg.getNoteNumber()] = i;

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

                    channelToNote[minIdx].insert(note.noteMsg.getNoteNumber());
                    channelToCcValue[minIdx] = note.ccMsg.getControllerValue();
                    noteToChannel[note.noteMsg.getNoteNumber()] = minIdx;

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
                    MidiMessage msg = MidiMessage::noteOff(note.noteMsg.getChannel(), noteNum).withTimeStamp(note.noteMsg.getTimeStamp());
                    noteSequences[minIdx]->addEvent(msg);
                    noteToChannel.erase(noteNum);
                }
                channelToNote[minIdx].clear();

                noteSequences[minIdx]->addEvent(note.noteMsg);
                ccSequences[minIdx]->addEvent(note.ccMsg);

                channelToNote[minIdx].insert(note.noteMsg.getNoteNumber());
                channelToCcValue[minIdx] = note.ccMsg.getControllerValue();
                noteToChannel[note.noteMsg.getNoteNumber()] = minIdx;

                channelUseCount[minIdx] = ++channelUseCounter;

                DBG("Override " << minIdx << " " << note.toString());
            }
        }
        else
        {
            if (noteToChannel.find(note.noteMsg.getNoteNumber()) != noteToChannel.end())
            {
                auto channel = noteToChannel[note.noteMsg.getNoteNumber()];
                noteSequences[channel]->addEvent(note.noteMsg);
                noteToChannel.erase(note.noteMsg.getNoteNumber());
                channelToNote[channel].erase(note.noteMsg.getNoteNumber());

                DBG("Note off " << channel << " " << note.toString());
            }
        }
    }
}

void VoiceScheduler::scheduleKontakt(const Array<InternalMidiMessage>& sequence,
    OwnedArray<MidiMessageSequence>& noteSequences, OwnedArray<MidiMessageSequence>& ccSequences, int n)
{
    std::unordered_map<int, std::unordered_set<int>> channelToNote;
    std::unordered_map<int, int> channelToCcValue;
    std::unordered_map<int, int> noteToChannel;

    int channelUseCounter = 0;
    std::map<int, int> channelUseCount;
    for (int i = 0; i < n * 12; i++)
    {
        channelToNote[i] = std::unordered_set<int>();
        channelToCcValue[i] = -2;
        channelUseCount[i] = 0;
    }

    for (auto& note : sequence)
    {
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

                    channelToNote[realIdx].insert(note.noteMsg.getNoteNumber());
                    noteToChannel[note.noteMsg.getNoteNumber()] = realIdx;

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

                    channelToNote[minIdx].insert(note.noteMsg.getNoteNumber());
                    channelToCcValue[minIdx] = note.ccMsg.getControllerValue();
                    noteToChannel[note.noteMsg.getNoteNumber()] = minIdx;

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
                for (auto noteNum : allNotesNow)
                {
                    MidiMessage msg = MidiMessage::noteOff(note.noteMsg.getChannel(), noteNum).withTimeStamp(note.noteMsg.getTimeStamp());
                    noteSequences[seqIndex]->addEvent(msg);
                    noteToChannel.erase(noteNum);
                }
                channelToNote[minIdx].clear();

                noteSequences[seqIndex]->addEvent(note.noteMsg);
                ccSequences[seqIndex]->addEvent(note.ccMsg);

                channelToNote[minIdx].insert(note.noteMsg.getNoteNumber());
                channelToCcValue[minIdx] = note.ccMsg.getControllerValue();
                noteToChannel[note.noteMsg.getNoteNumber()] = minIdx;

                channelUseCount[minIdx] = ++channelUseCounter;

                DBG("Override " << seqIndex << " " << minIdx % 12 << note.toString());
            }
        }
        else
        {
            if (noteToChannel.find(note.noteMsg.getNoteNumber()) != noteToChannel.end())
            {
                auto channel = noteToChannel[note.noteMsg.getNoteNumber()];
                noteSequences[channel / 12]->addEvent(note.noteMsg);
                noteToChannel.erase(note.noteMsg.getNoteNumber());
                channelToNote[channel].erase(note.noteMsg.getNoteNumber());

                DBG("Note off " << channel / 12 << " " << channel % 12 << note.toString());
            }
        }
    }
}
