#include "Project.h"
#include "PluginProcessor.h"
#include "ProjectListener.h"

#include "MidiTrack.h"
#include "NoteTrack.h"
#include "TempoTrack.h"
#include "TimeSignatureTrack.h"

Project::Project(MicroChromoAudioProcessor& p, String title) :
    FileBasedDocument(".mcmproj", "*.mcmproj", "Open", "Save"),
    processor(p), undoManager(p.getUndoManager()), _title(title) {}

std::unique_ptr<XmlElement> Project::createXml()
{
    return nullptr;
}

void Project::loadFromXml(XmlElement* xml)
{

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
void Project::broadcastChangeEvent(const MidiEvent& oldEvent, const MidiEvent& newEvent)
{
    jassert(newEvent.isValid());
    changeListeners.call(&ProjectListener::onChangeMidiEvent, oldEvent, newEvent);
    sendChangeMessage();
}

void Project::broadcastAddEvent(const MidiEvent& event)
{
    jassert(event.isValid());
    changeListeners.call(&ProjectListener::onAddMidiEvent, event);
    sendChangeMessage();
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

void Project::broadcastChangeTrackProperties(MidiTrack* const track)
{
    changeListeners.call(&ProjectListener::onChangeTrackProperties, track);
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
