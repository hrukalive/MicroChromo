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

#include "MidiTrack.h"
#include "Note.h"

class Project;

//==============================================================================
class NoteTrack final : public MidiTrack
{
public:
    NoteTrack(Project& project) noexcept;
    NoteTrack(Project& project, const String& name, int channel) noexcept;

    //===------------------------------------------------------------------===//
    // Undoable track editing
    //===------------------------------------------------------------------===//
    MidiEvent* insert(const Note& note, bool undoable);
    bool remove(const Note& note, bool undoable);
    bool change(const Note& note, const Note& newNote, bool undoable);

    bool insertGroup(Array<Note>& notes, bool undoable);
    bool removeGroup(Array<Note>& notes, bool undoable);
    bool changeGroup(Array<Note>& eventsBefore, Array<Note>& eventsAfter, bool undoable);

    //===------------------------------------------------------------------===//
    // Accessors
    //===------------------------------------------------------------------===//
    float getLastBeat() const noexcept override;

    inline Note* operator[] (int index) const noexcept
    {
        return dynamic_cast<Note*>(midiEvents[index]);
    }

    //===------------------------------------------------------------------===//
    // Serializable
    //===------------------------------------------------------------------===//
    ValueTree serialize() const override;
    ValueTree serializeWithId() const;
    void deserialize(const ValueTree& tree) override;
    void reset() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoteTrack);
    JUCE_DECLARE_WEAK_REFERENCEABLE(NoteTrack);
};
