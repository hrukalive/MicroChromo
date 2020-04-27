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
#include "TempoMarkerEvent.h"

class Project;

//===----------------------------------------------------------------------===//
// Insert
//===----------------------------------------------------------------------===//
class TempoMarkerEventInsertAction final : public UndoableAction
{
public:

    explicit TempoMarkerEventInsertAction(Project& project) noexcept : project(project) {}
    TempoMarkerEventInsertAction(Project& project, IdGenerator::Id trackId, const TempoMarkerEvent& target) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;

private:
    Project& project;
    IdGenerator::Id trackId;
    TempoMarkerEvent event;

    JUCE_DECLARE_NON_COPYABLE(TempoMarkerEventInsertAction)
};

//===----------------------------------------------------------------------===//
// Remove
//===----------------------------------------------------------------------===//
class TempoMarkerEventRemoveAction final : public UndoableAction
{
public:

    explicit TempoMarkerEventRemoveAction(Project& project) noexcept : project(project) {}
    TempoMarkerEventRemoveAction(Project& project,
        IdGenerator::Id trackId, const TempoMarkerEvent& target) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;

private:
    Project& project;
    IdGenerator::Id trackId;
    TempoMarkerEvent event;

    JUCE_DECLARE_NON_COPYABLE(TempoMarkerEventRemoveAction)
};

//===----------------------------------------------------------------------===//
// Change
//===----------------------------------------------------------------------===//
class TempoMarkerEventChangeAction final : public UndoableAction
{
public:

    explicit TempoMarkerEventChangeAction(Project& project) noexcept : project(project) {}
    TempoMarkerEventChangeAction(Project& project, IdGenerator::Id trackId,
        const TempoMarkerEvent& target, const TempoMarkerEvent& newParameters) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;
    UndoableAction* createCoalescedAction(UndoableAction* nextAction) override;

private:
    Project& project;
    IdGenerator::Id trackId;

    TempoMarkerEvent eventBefore;
    TempoMarkerEvent eventAfter;

    JUCE_DECLARE_NON_COPYABLE(TempoMarkerEventChangeAction)
};

//===----------------------------------------------------------------------===//
// Insert Group
//===----------------------------------------------------------------------===//
class TempoMarkerEventsGroupInsertAction final : public UndoableAction
{
public:

    explicit TempoMarkerEventsGroupInsertAction(Project& project) noexcept : project(project) {}
    TempoMarkerEventsGroupInsertAction(Project& project,
        IdGenerator::Id trackId, Array<TempoMarkerEvent>& target) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;

private:
    Project& project;
    IdGenerator::Id trackId;
    Array<TempoMarkerEvent> signatures;

    JUCE_DECLARE_NON_COPYABLE(TempoMarkerEventsGroupInsertAction)
};

//===----------------------------------------------------------------------===//
// Remove Group
//===----------------------------------------------------------------------===//
class TempoMarkerEventsGroupRemoveAction final : public UndoableAction
{
public:

    explicit TempoMarkerEventsGroupRemoveAction(Project& project) noexcept :
        project(project) {}
    TempoMarkerEventsGroupRemoveAction(Project& project,
        IdGenerator::Id trackId, Array<TempoMarkerEvent>& target) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;

private:
    Project& project;
    IdGenerator::Id trackId;
    Array<TempoMarkerEvent> signatures;

    JUCE_DECLARE_NON_COPYABLE(TempoMarkerEventsGroupRemoveAction)
};

//===----------------------------------------------------------------------===//
// Change Group
//===----------------------------------------------------------------------===//
class TempoMarkerEventsGroupChangeAction final : public UndoableAction
{
public:

    explicit TempoMarkerEventsGroupChangeAction(Project& project) noexcept :
        project(project) {}
    TempoMarkerEventsGroupChangeAction(Project& project, IdGenerator::Id trackId,
        Array<TempoMarkerEvent>& state1, Array<TempoMarkerEvent>& state2)  noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;
    UndoableAction* createCoalescedAction(UndoableAction* nextAction) override;

private:
    Project& project;
    IdGenerator::Id trackId;

    Array<TempoMarkerEvent> eventsBefore;
    Array<TempoMarkerEvent> eventsAfter;

    JUCE_DECLARE_NON_COPYABLE(TempoMarkerEventsGroupChangeAction)
};
