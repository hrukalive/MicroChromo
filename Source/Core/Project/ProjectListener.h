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

#pragma once

class MidiEvent;
class MidiTrack;
class PitchColorMapEntry;
class PitchColorMap;

//==============================================================================
class ProjectListener
{
public:

    ProjectListener() {}
    virtual ~ProjectListener() {}

    virtual void onAddMidiEvent(const MidiEvent &event) = 0;
    virtual void onPostAddMidiEvent() = 0;
    virtual void onChangeMidiEvent(const MidiEvent &oldEvent, const MidiEvent &newEvent) = 0;
    virtual void onPostChangeMidiEvent() = 0;
    virtual void onRemoveMidiEvent(const MidiEvent &event) = 0;
    virtual void onPostRemoveMidiEvent(MidiTrack *const layer) {}

    virtual void onAddTrack(MidiTrack *const track) = 0;
    virtual void onChangeTrackProperties(MidiTrack *const track) = 0;
    virtual void onRemoveTrack(MidiTrack* const track) = 0;
    virtual void onPostRemoveTrack() = 0;

    virtual void onAddPitchColorMapEntry(const PitchColorMapEntry& entry) = 0;
    virtual void onPostAddPitchColorMapEntry() = 0;
    virtual void onChangePitchColorMapEntry(const PitchColorMapEntry& oldEntry, const PitchColorMapEntry& newEntry) = 0;
    virtual void onPostChangePitchColorMapEntry() = 0;
    virtual void onRemovePitchColorMapEntry(const PitchColorMapEntry& entry) = 0;
    virtual void onPostRemovePitchColorMapEntry() = 0;
    virtual void onChangePitchColorMap(PitchColorMap* const colorMap) = 0;

    virtual void onChangeProjectBeatRange(float firstBeat, float lastBeat) = 0;
    virtual void onChangeViewBeatRange(float firstBeat, float lastBeat) = 0;

    virtual void onReloadProjectContent(const Array<MidiTrack*>& tracks) = 0;

};
