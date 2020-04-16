#include "Project.h"
#include "PluginProcessor.h"
#include "ProjectListener.h"

#include "ProjectActions.h"

Project::Project(MicroChromoAudioProcessor& p, String title) :
    FileBasedDocument(".mcmproj", "*.mcmproj", "Open", "Save"),
    processor(p), undoManager(p.getUndoManager()), _title(title)
{
    tempoMarkerTrack.reset(new TempoTrack(*this));
    tempoMarkerTrack->insert(TempoMarkerEvent(tempoMarkerTrack.get()), false);
    timeSignatureTrack.reset(new TimeSignatureTrack(*this));
    timeSignatureTrack->insert(TimeSignatureEvent(timeSignatureTrack.get()), false);
    addTrack(ValueTree(), "empty", 1, false);
    pitchColorMap.reset(new PitchColorMap(*this));
}

std::unique_ptr<XmlElement> Project::createXml()
{
    return nullptr;
}

void Project::loadFromXml(XmlElement* xml)
{

}

MidiTrack* Project::addTrack(ValueTree& serializedState, const String& name, int channel, bool undoable)
{
    if (undoable)
    {
        getUndoManager().
            perform(new NoteTrackInsertAction(*this, serializedState, name, channel));
    }
    else
    {
        auto* track = new NoteTrack(*this, name, channel);
        track->deserialize(serializedState);
        noteTracks.addSorted(*track, track);
        broadcastAddTrack(track);
        return track;
    }
    return nullptr;
}

bool Project::removeTrack(IdGenerator::Id trackId, bool undoable)
{
    if (undoable)
    {
        getUndoManager().
            perform(new NoteTrackRemoveAction(*this, trackId));
    }
    else
    {
        auto* track = findTrackById<NoteTrack>(trackId);
        if (track == nullptr)
            return false;
        const int index = noteTracks.indexOfSorted(*track, track);
        if (index == -1)
            return false;
        broadcastRemoveTrack(track);
        noteTracks.remove(index, true);
        broadcastPostRemoveTrack();
    }
    return true;
}

Array<MidiTrack*> Project::getTracks() const
{
    Array<MidiTrack*> tracks;

    for (auto* track : noteTracks)
        tracks.add(track);
    tracks.add(tempoMarkerTrack.get());
    tracks.add(timeSignatureTrack.get());

    return tracks;
}

Point<float> Project::getProjectRangeInBeats() const
{
    float lastBeat = -FLT_MAX;
    float firstBeat = FLT_MAX;

    for (const auto* track : noteTracks)
    {
        const float sequenceFirstBeat = track->getFirstBeat();
        const float sequenceLastBeat = track->getLastBeat();
        firstBeat = jmin(firstBeat, sequenceFirstBeat + 0);
        lastBeat = jmax(lastBeat, sequenceLastBeat + 0);
    }

    const float defaultNumBeats = DEFAULT_NUM_BARS * BEATS_PER_BAR;

    if (firstBeat == FLT_MAX)
        firstBeat = 0;
    else if (firstBeat > lastBeat)
        firstBeat = lastBeat - defaultNumBeats;

    if ((lastBeat - firstBeat) < defaultNumBeats)
        lastBeat = firstBeat + defaultNumBeats;

    return { firstBeat, lastBeat };
}

MidiTrack* Project::getTrackById(IdGenerator::Id id)
{
    if (tempoMarkerTrack->getTrackId() == id)
        return tempoMarkerTrack.get();
    else if (timeSignatureTrack->getTrackId() == id)
        return timeSignatureTrack.get();
    else
    {
        for (auto* track : noteTracks)
            if (track->getTrackId() == id)
                return track;
    }
    return nullptr;
}

MidiTrack* Project::getTrackByName(const String& name)
{
    if (tempoMarkerTrack->getTrackName() == name)
        return tempoMarkerTrack.get();
    else if (timeSignatureTrack->getTrackName() == name)
        return timeSignatureTrack.get();
    else
    {
        for (auto* track : noteTracks)
            if (track->getTrackName() == name)
                return track;
    }
    return nullptr;
}

//===----------------------------------------------------------------------===//
// ProjectListeners management
//===----------------------------------------------------------------------===//
void Project::addListener(ProjectListener* listener)
{
    jassert(MessageManager::getInstance()->currentThreadHasLockedMessageManager());
    changeListeners.add(listener);
}

void Project::removeListener(ProjectListener* listener)
{
    jassert(MessageManager::getInstance()->currentThreadHasLockedMessageManager());
    changeListeners.remove(listener);
}

void Project::removeAllListeners()
{
    jassert(MessageManager::getInstance()->currentThreadHasLockedMessageManager());
    changeListeners.clear();
}


//===----------------------------------------------------------------------===//
// Broadcaster
//===----------------------------------------------------------------------===//
void Project::broadcastAddEvent(const MidiEvent& event)
{
    jassert(event.isValid());
    changeListeners.call(&ProjectListener::onAddMidiEvent, event);
    sendChangeMessage();
}

void Project::broadcastPostAddEvent()
{
    changeListeners.call(&ProjectListener::onPostAddMidiEvent);
}

void Project::broadcastChangeEvent(const MidiEvent& oldEvent, const MidiEvent& newEvent)
{
    jassert(newEvent.isValid());
    changeListeners.call(&ProjectListener::onChangeMidiEvent, oldEvent, newEvent);
    sendChangeMessage();
}

void Project::broadcastPostChangeEvent()
{
    changeListeners.call(&ProjectListener::onPostChangeMidiEvent);
}

void Project::broadcastRemoveEvent(const MidiEvent& event)
{
    jassert(event.isValid());
    changeListeners.call(&ProjectListener::onRemoveMidiEvent, event);
    sendChangeMessage();
}

void Project::broadcastPostRemoveEvent(MidiTrack* const layer)
{
    changeListeners.call(&ProjectListener::onPostRemoveMidiEvent, layer);
    sendChangeMessage();
}

void Project::broadcastAddTrack(MidiTrack* const track)
{
    changeListeners.call(&ProjectListener::onAddTrack, track);
    sendChangeMessage();
}

void Project::broadcastRemoveTrack(MidiTrack* const track)
{
    changeListeners.call(&ProjectListener::onRemoveTrack, track);
    sendChangeMessage();
}

void Project::broadcastPostRemoveTrack()
{
    changeListeners.call(&ProjectListener::onPostRemoveTrack);
    sendChangeMessage();
}

void Project::broadcastChangeTrackProperties(MidiTrack* const track)
{
    changeListeners.call(&ProjectListener::onChangeTrackProperties, track);
    sendChangeMessage();
}

void Project::broadcastAddPitchColorMapEntry(const PitchColorMapEntry& entry)
{
    changeListeners.call(&ProjectListener::onAddPitchColorMapEntry, entry);
    sendChangeMessage();
}

void Project::broadcastPostAddPitchColorMapEntry()
{
    changeListeners.call(&ProjectListener::onPostAddPitchColorMapEntry);
}

void Project::broadcastChangePitchColorMapEntry(const PitchColorMapEntry& oldEntry,
    const PitchColorMapEntry& newEntry)
{
    changeListeners.call(&ProjectListener::onChangePitchColorMapEntry, oldEntry, newEntry);
    sendChangeMessage();
}

void Project::broadcastPostChangePitchColorMapEntry()
{
    changeListeners.call(&ProjectListener::onPostChangePitchColorMapEntry);
}

void Project::broadcastRemovePitchColorMapEntry(const PitchColorMapEntry& entry)
{
    changeListeners.call(&ProjectListener::onRemovePitchColorMapEntry, entry);
    sendChangeMessage();
}

void Project::broadcastPostRemovePitchColorMapEntry()
{
    changeListeners.call(&ProjectListener::onPostRemovePitchColorMapEntry);
    sendChangeMessage();
}

Point<float> Project::broadcastChangeProjectBeatRange()
{
    const auto beatRange = getProjectRangeInBeats();
    const float firstBeat = beatRange.getX();
    const float lastBeat = beatRange.getY();

    //transport->onChangeProjectBeatRange(firstBeat, lastBeat);
    //changeListeners.callExcluding(transport, &ProjectListener::onChangeProjectBeatRange, firstBeat, lastBeat);
    sendChangeMessage();

    return beatRange;
}

void Project::broadcastReloadProjectContent()
{
    changeListeners.call(&ProjectListener::onReloadProjectContent, getTracks());
    sendChangeMessage();
}

void Project::broadcastChangeViewBeatRange(float firstBeat, float lastBeat)
{
    changeListeners.call(&ProjectListener::onChangeViewBeatRange, firstBeat, lastBeat);
    // sendChangeMessage(); the project itself didn't change, so dont call this
}

void Project::sort()
{
    if (noteTracks.size() > 0)
    {
        noteTracks.sort(*noteTracks.getFirst(), true);
    }
}

String Project::getDocumentTitle()
{
    return _title;
}

Result Project::loadDocument(const File& file)
{
    return Result::ok();
}

Result Project::saveDocument(const File& file)
{
    return Result::ok();
}

File Project::getLastDocumentOpened()
{
    return File();
}

void Project::setLastDocumentOpened(const File& file)
{

}
