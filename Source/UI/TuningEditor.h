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

#include "Common.h"
#include "Project.h"
#include "ProjectListener.h"

class MicroChromoAudioProcessorEditor;

 //==============================================================================
class TuningEditor : public Component, public ProjectListener
{
public:
    TuningEditor(MicroChromoAudioProcessorEditor& editor);
    ~TuningEditor();

    //===------------------------------------------------------------------===//
    // Components
    //===------------------------------------------------------------------===//
    void paint(Graphics& g) override;
    void resized() override;

    //===------------------------------------------------------------------===//
    // Project Listener
    //===------------------------------------------------------------------===//
    void onAddMidiEvent(const MidiEvent& event) override {}
    void onPostAddMidiEvent() override {}
    void onChangeMidiEvent(const MidiEvent& oldEvent, const MidiEvent& newEvent) override {}
    void onPostChangeMidiEvent() override {}
    void onRemoveMidiEvent(const MidiEvent& event) override {}
    void onPostRemoveMidiEvent(MidiTrack* const layer) {}

    void onAddTrack(MidiTrack* const track) override {}
    void onChangeTrackProperties(MidiTrack* const track) override {}
    void onRemoveTrack(MidiTrack* const track) override {}
    void onPostRemoveTrack() override {}

    void onAddPitchColorMapEntry(const PitchColorMapEntry& entry) override {}
    void onPostAddPitchColorMapEntry() override {}
    void onChangePitchColorMapEntry(const PitchColorMapEntry& oldEntry, const PitchColorMapEntry& newEntry) override {}
    void onPostChangePitchColorMapEntry() override {}
    void onRemovePitchColorMapEntry(const PitchColorMapEntry& entry) override {}
    void onPostRemovePitchColorMapEntry() override {}
    void onChangePitchColorMap(PitchColorMap* const colorMap) override {}

    void onPostTuningChange() override;

    void onChangeProjectBeatRange(float firstBeat, float lastBeat) override {}
    void onChangeViewBeatRange(float firstBeat, float lastBeat) override {}

    void onReloadProjectContent(const Array<MidiTrack*>& tracks) override;

private:
    //==============================================================================
    MicroChromoAudioProcessorEditor& owner;
    MicroChromoAudioProcessor& processor;
    Project& project;

    //==============================================================================
    Label noteLabels[12], freqALabel;
    Slider noteSliders[12], freqASlider;
    TextButton resetBtn{ "Reset" };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TuningEditor)
};
