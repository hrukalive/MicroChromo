#pragma once

#include "Common.h"
#include "TimeSignatureEvent.h"

class Project;

//===----------------------------------------------------------------------===//
// Insert
//===----------------------------------------------------------------------===//
class TimeSignatureEventInsertAction final : public UndoableAction
{
public:

    explicit TimeSignatureEventInsertAction(Project& project) noexcept : project(project) {}
    TimeSignatureEventInsertAction(Project& project, IdGenerator::Id trackId, const TimeSignatureEvent& target) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;

private:
    Project& project;
    IdGenerator::Id trackId;
    TimeSignatureEvent event;

    JUCE_DECLARE_NON_COPYABLE(TimeSignatureEventInsertAction)
};

//===----------------------------------------------------------------------===//
// Remove
//===----------------------------------------------------------------------===//
class TimeSignatureEventRemoveAction final : public UndoableAction
{
public:

    explicit TimeSignatureEventRemoveAction(Project& project) noexcept : project(project) {}
    TimeSignatureEventRemoveAction(Project& project, 
        IdGenerator::Id trackId, const TimeSignatureEvent& target) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;

private:
    Project& project;
    IdGenerator::Id trackId;
    TimeSignatureEvent event;

    JUCE_DECLARE_NON_COPYABLE(TimeSignatureEventRemoveAction)
};

//===----------------------------------------------------------------------===//
// Change
//===----------------------------------------------------------------------===//
class TimeSignatureEventChangeAction final : public UndoableAction
{
public:

    explicit TimeSignatureEventChangeAction(Project& project) noexcept : project(project) {}
    TimeSignatureEventChangeAction(Project& project, IdGenerator::Id trackId,
        const TimeSignatureEvent& target, const TimeSignatureEvent& newParameters) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;
    UndoableAction* createCoalescedAction(UndoableAction* nextAction) override;

private:
    Project& project;
    IdGenerator::Id trackId;

    TimeSignatureEvent eventBefore;
    TimeSignatureEvent eventAfter;

    JUCE_DECLARE_NON_COPYABLE(TimeSignatureEventChangeAction)
};

//===----------------------------------------------------------------------===//
// Insert Group
//===----------------------------------------------------------------------===//
class TimeSignatureEventsGroupInsertAction final : public UndoableAction
{
public:

    explicit TimeSignatureEventsGroupInsertAction(Project& project) noexcept : project(project) {}
    TimeSignatureEventsGroupInsertAction(Project& project,
        IdGenerator::Id trackId, Array<TimeSignatureEvent>& target) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;

private:
    Project& project;
    IdGenerator::Id trackId;
    Array<TimeSignatureEvent> signatures;

    JUCE_DECLARE_NON_COPYABLE(TimeSignatureEventsGroupInsertAction)
};

//===----------------------------------------------------------------------===//
// Remove Group
//===----------------------------------------------------------------------===//
class TimeSignatureEventsGroupRemoveAction final : public UndoableAction
{
public:

    explicit TimeSignatureEventsGroupRemoveAction(Project& project) noexcept :
        project(project) {}
    TimeSignatureEventsGroupRemoveAction(Project& project,
        IdGenerator::Id trackId, Array<TimeSignatureEvent>& target) noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;

private:
    Project& project;
    IdGenerator::Id trackId;
    Array<TimeSignatureEvent> signatures;

    JUCE_DECLARE_NON_COPYABLE(TimeSignatureEventsGroupRemoveAction)
};

//===----------------------------------------------------------------------===//
// Change Group
//===----------------------------------------------------------------------===//
class TimeSignatureEventsGroupChangeAction final : public UndoableAction
{
public:

    explicit TimeSignatureEventsGroupChangeAction(Project& project) noexcept :
        project(project) {}
    TimeSignatureEventsGroupChangeAction(Project& project, IdGenerator::Id trackId,
        Array<TimeSignatureEvent>& state1, Array<TimeSignatureEvent>& state2)  noexcept;

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override;
    UndoableAction* createCoalescedAction(UndoableAction* nextAction) override;

private:
    Project& project;
    IdGenerator::Id trackId;

    Array<TimeSignatureEvent> eventsBefore;
    Array<TimeSignatureEvent> eventsAfter;

    JUCE_DECLARE_NON_COPYABLE(TimeSignatureEventsGroupChangeAction)
};
