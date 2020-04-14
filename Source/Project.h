#pragma once

#define DEFAULT_NUM_BARS 8

#include "Common.h"

class ProjectListener;
class MidiTrack;
class NoteTrack;
class TempoTrack;
class TimeSignatureTrack;

class MicroChromoAudioProcessor;

//==============================================================================
class Project : public FileBasedDocument
{
public:
    //==============================================================================
    Project(MicroChromoAudioProcessor& p, String title);
    ~Project() = default;

    //==============================================================================
    auto& getNoteColorMap() { return noteColorMap; }

    //==============================================================================
    std::unique_ptr<XmlElement> createXml();
    void loadFromXml(XmlElement* xml);

    Array<MidiTrack*> getTracks() const;
    Point<float> getProjectRangeInBeats() const;
    UndoManager& getUndoManager() noexcept { return undoManager; }

    //===------------------------------------------------------------------===//
    // Project listeners
    //===------------------------------------------------------------------===//
    void addListener(ProjectListener* listener);
    void removeListener(ProjectListener* listener);
    void removeAllListeners();

    //===------------------------------------------------------------------===//
    // Broadcaster
    //===------------------------------------------------------------------===//
    void broadcastAddEvent(const MidiEvent& event);
    void broadcastChangeEvent(const MidiEvent& oldEvent, const MidiEvent& newEvent);
    void broadcastRemoveEvent(const MidiEvent& event);
    void broadcastPostRemoveEvent(MidiTrack* const layer);

    void broadcastAddTrack(MidiTrack* const track);
    void broadcastRemoveTrack(MidiTrack* const track);
    void broadcastChangeTrackProperties(MidiTrack* const track);

    void broadcastChangeViewBeatRange(float firstBeat, float lastBeat);
    void broadcastReloadProjectContent();
    Point<float> broadcastChangeProjectBeatRange();

private:
    //==============================================================================
    String getDocumentTitle() override;
    Result loadDocument(const File& file) override;
    Result saveDocument(const File& file) override;
    File getLastDocumentOpened() override;
    void setLastDocumentOpened(const File& file) override;

    //==============================================================================
    MicroChromoAudioProcessor& processor;
    UndoManager& undoManager;

    //==============================================================================
    ListenerList<ProjectListener> changeListeners;
    String _title;

    //==============================================================================
    std::unique_ptr<TempoTrack> tempoMarkerTrack{ nullptr };
    std::unique_ptr<TimeSignatureTrack> timeSignatureTrack{ nullptr };
    OwnedArray<NoteTrack> noteTracks;
    HashMap<String, ColorPitchBendRecord> noteColorMap;
};

