#pragma once

#define DEFAULT_NUM_BARS 8

#include "Common.h"

#include "MidiEvent.h"
#include "NoteTrack.h"
#include "TempoTrack.h"
#include "TimeSignatureTrack.h"
#include "PitchColorMap.h"

class ProjectListener;
class MicroChromoAudioProcessor;

//==============================================================================
class Project : public FileBasedDocument
{
public:
    //==============================================================================
    Project(MicroChromoAudioProcessor& p, String title);
    ~Project() = default;

    //==============================================================================
    std::unique_ptr<XmlElement> createXml();
    void loadFromXml(XmlElement* xml);

    //===------------------------------------------------------------------===//
    // Accessors
    //===------------------------------------------------------------------===//
    MidiTrack* addTrack(ValueTree& serializedState, const String& name, int channel, bool undoable);
    bool removeTrack(IdGenerator::Id trackId, bool undoable);

    Array<MidiTrack*> getTracks() const;
    Point<float> getProjectRangeInBeats() const;
    UndoManager& getUndoManager() noexcept { return undoManager; }

    TempoTrack* getTempoTrack() { return tempoMarkerTrack.get(); }
    TimeSignatureTrack* getTimeSignatureTrack() { return timeSignatureTrack.get(); }
    const OwnedArray<NoteTrack>& getNoteTracks() { return noteTracks; }
    PitchColorMap* getPitchColorMap() { return pitchColorMap.get(); }

    template<typename T>
    T* findTrackById(IdGenerator::Id id)
    {
        return dynamic_cast<T*>(getTrackById(id));
    }

    template<typename T>
    T* findTrackByName(const String& name)
    {
        return dynamic_cast<T*>(getTrackByName(name));
    }

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
    void broadcastPostAddEvent();
    void broadcastChangeEvent(const MidiEvent& oldEvent, const MidiEvent& newEvent);
    void broadcastPostChangeEvent();
    void broadcastRemoveEvent(const MidiEvent& event);
    void broadcastPostRemoveEvent(MidiTrack* const layer);

    void broadcastAddTrack(MidiTrack* const track);
    void broadcastRemoveTrack(MidiTrack* const track);
    void broadcastPostRemoveTrack();
    void broadcastChangeTrackProperties(MidiTrack* const track);

    void broadcastAddPitchColorMapEntry(const PitchColorMapEntry& entry);
    void broadcastPostAddPitchColorMapEntry();
    void broadcastChangePitchColorMapEntry(const PitchColorMapEntry& oldEntry, 
        const PitchColorMapEntry& newEntry);
    void broadcastPostChangePitchColorMapEntry();
    void broadcastRemovePitchColorMapEntry(const PitchColorMapEntry& entry);
    void broadcastPostRemovePitchColorMapEntry();

    void broadcastChangeViewBeatRange(float firstBeat, float lastBeat);
    void broadcastReloadProjectContent();
    Point<float> broadcastChangeProjectBeatRange();

    //===------------------------------------------------------------------===//
    // OwnedArray wrapper
    //===------------------------------------------------------------------===//
    void sort();

    inline int size() const noexcept
    {
        return noteTracks.size();
    }

    inline NoteTrack* const* begin() const noexcept
    {
        return noteTracks.begin();
    }

    inline NoteTrack* const* end() const noexcept
    {
        return noteTracks.end();
    }

    inline NoteTrack* getUnchecked(const int index) const noexcept
    {
        return noteTracks.getUnchecked(index);
    }

    inline int indexOfSorted(const NoteTrack* const track) const noexcept
    {
        jassert(noteTracks[noteTracks.indexOfSorted(*track, track)] == track);
        noteTracks.indexOfSorted(*track, track);
    }

private:
    friend class NoteTrackInsertAction;

    //==============================================================================
    String getDocumentTitle() override;
    Result loadDocument(const File& file) override;
    Result saveDocument(const File& file) override;
    File getLastDocumentOpened() override;
    void setLastDocumentOpened(const File& file) override;

    //==============================================================================
    MidiTrack* getTrackById(IdGenerator::Id id);
    MidiTrack* getTrackByName(const String& name);

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
    std::unique_ptr<PitchColorMap> pitchColorMap;
};

