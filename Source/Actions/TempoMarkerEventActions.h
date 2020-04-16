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
