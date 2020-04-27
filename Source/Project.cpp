#include "Project.h"
#include "PluginProcessor.h"
#include "ProjectListener.h"

#include "ProjectActions.h"

Project::Project(MicroChromoAudioProcessor& p, String title) :
    processor(p), undoManager(p.getUndoManager()), _title(title)
{
    tempoMarkerTrack.reset(new TempoTrack(*this));
    tempoMarkerTrack->insert(TempoMarkerEvent(tempoMarkerTrack.get()), false);
    timeSignatureTrack.reset(new TimeSignatureTrack(*this));
    timeSignatureTrack->insert(TimeSignatureEvent(timeSignatureTrack.get()), false);
    addTrack(ValueTree(), "empty", 1, false);
    pitchColorMap.reset(new PitchColorMap(this));

    voiceScheduler.reset(new VoiceScheduler(*this));
}

NoteTrack* Project::addTrack(ValueTree& serializedState, const String& name, int channel, bool undoable)
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

Array<MidiTrack*> Project::getAllTracks() const
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

void Project::broadcastChangePitchColorMap(PitchColorMap* const colorMap)
{
    changeListeners.call(&ProjectListener::onChangePitchColorMap, colorMap);
    sendChangeMessage();
}

Point<float> Project::broadcastChangeProjectBeatRange()
{
    const auto beatRange = getProjectRangeInBeats();
    const float firstBeat = beatRange.getX();
    const float lastBeat = beatRange.getY();

    //changeListeners.callExcluding(&ProjectListener::onChangeProjectBeatRange, firstBeat, lastBeat);
    sendChangeMessage();

    return beatRange;
}

void Project::broadcastReloadProjectContent()
{
    changeListeners.call(&ProjectListener::onReloadProjectContent, getAllTracks());
    sendChangeMessage();
}

void Project::broadcastChangeViewBeatRange(float firstBeat, float lastBeat)
{
    changeListeners.call(&ProjectListener::onChangeViewBeatRange, firstBeat, lastBeat);
    // sendChangeMessage(); the project itself didn't change, so dont call this
}

//===------------------------------------------------------------------===//
// Serializable
//===------------------------------------------------------------------===//
ValueTree Project::serialize() const
{
    ValueTree tree(Serialization::Core::project);
    tree.setProperty(Serialization::Core::projectName, _title, nullptr);
    tree.appendChild(tempoMarkerTrack->serialize(), nullptr);
    tree.appendChild(timeSignatureTrack->serialize(), nullptr);
    tree.appendChild(pitchColorMap->serialize(), nullptr);
    for (auto* track : noteTracks)
        tree.appendChild(track->serialize(), nullptr);
    return tree;
}

void Project::deserialize(const ValueTree& tree)
{
    reset();

    const auto root =
        tree.hasType(Serialization::Core::project) ?
        tree : tree.getChildWithName(Serialization::Core::project);

    if (!root.isValid())
        return;

    _title = root.getProperty(Serialization::Core::projectName, "Untitled");
    tempoMarkerTrack->deserialize(tree);
    timeSignatureTrack->deserialize(tree);
    pitchColorMap->deserialize(tree);
    for (const auto& e : root)
    {
        if (e.hasType(Serialization::Midi::notes))
        {
            auto* track = new NoteTrack(*this);
            track->deserialize(e);

            noteTracks.addSorted(*track, track);
        }
    }
}

void Project::reset()
{
    noteTracks.clear();
}

//===------------------------------------------------------------------===//
// OwnedArray wrapper
//===------------------------------------------------------------------===//
void Project::sort()
{
    if (noteTracks.size() > 0)
    {
        noteTracks.sort(*noteTracks.getFirst(), true);
    }
}

//===------------------------------------------------------------------===//
// Helpers
//===------------------------------------------------------------------===//
std::pair<int, float> Project::getBarAndBeat(float timestamp)
{
    int lastNumerator = 4, lastDenominator = 4;
    float lastPos = 0;
    int barElapsed = 0;
    for (int i = 0; i < timeSignatureTrack->size(); i++)
    {
        auto* sig = (*timeSignatureTrack)[i];
        float beatDiff = sig->getBeat() - lastPos;
        if (timestamp - beatDiff >= 0)
            timestamp -= beatDiff;
        else
            break;
        barElapsed += ceilf(beatDiff * lastDenominator / 4.0f / lastNumerator);
        lastPos = sig->getBeat();
        lastNumerator = sig->getNumerator();
        lastDenominator = sig->getDenominator();
    }
    barElapsed += floorf(timestamp * lastDenominator / 4.0f / lastNumerator);
    float beat = fmodf(timestamp * lastDenominator / 4.0f, (float)lastNumerator);
    return std::make_pair(barElapsed + 1, beat + 1);
}

Array<MidiFile> Project::exportMidiFiles()
{
    OwnedArray<MidiMessageSequence> noteSeqs, ccSeqs;
    for (int i = 0; i < processor.getNumInstances(); i++)
    {
        noteSeqs.add(new MidiMessageSequence());
        ccSeqs.add(new MidiMessageSequence());
    }

    voiceScheduler->schedule(noteSeqs, ccSeqs, processor.getNumInstances(),
        processor.getCcBase(), processor.getPitchShiftModulationSource() == USE_KONTAKT,
        processor.getBlockSize() / processor.getSampleRate(), 960);

    Array<MidiFile> files;
    for (int i = 0; i < processor.getNumInstances(); i++)
    {
        if (noteSeqs[i]->getNumEvents() == 0)
            continue;

        files.add(MidiFile());

        auto& file = files.getReference(files.size() - 1);
        file.setTicksPerQuarterNote(960);

        MidiMessageSequence seq;
        for (auto* evt : *tempoMarkerTrack)
            if (auto* marker = dynamic_cast<TempoMarkerEvent*>(evt))
                seq.addEvent(MidiMessage::tempoMetaEvent(int((60000.f / marker->getBPM()) * 1000.f)).withTimeStamp((int)(marker->getBeat() * 960)));
        for (auto* evt : *timeSignatureTrack)
            if (auto* sig = dynamic_cast<TimeSignatureEvent*>(evt))
                seq.addEvent(MidiMessage::timeSignatureMetaEvent(sig->getNumerator(), sig->getDenominator()).withTimeStamp((int)(sig->getBeat() * 960)));
        for (auto* evt : *noteSeqs[i])
        {
            if (evt->message.getChannel() == 0)
                jassertfalse;
            seq.addEvent(MidiMessage(evt->message).withTimeStamp(evt->message.getTimeStamp()));
        }
        for (auto* evt : *ccSeqs[i])
            seq.addEvent(MidiMessage(evt->message).withTimeStamp(evt->message.getTimeStamp()));
        seq.sort();
        seq.updateMatchedPairs();
        seq.addEvent(MidiMessage::endOfTrack(), noteSeqs[i]->getEndTime() * 960);

        file.addTrack(seq);
    }
    return files;
}

void Project::loadMidiFile(const Array<File>& files)
{
    std::shared_ptr<OwnedArray<MidiFile>> midiFiles{ new OwnedArray<MidiFile>() };
    std::shared_ptr<StringArray> midiFilenames{ new StringArray() };
    for (auto& file : files)
    {
        auto* midiFile = new MidiFile();
        std::unique_ptr<InputStream> stream{ new FileInputStream(file) };
        midiFile->readFrom(*stream);

        if (midiFile->getTimeFormat() > 0 && midiFile->getNumTracks() > 0)
        {
            midiFiles->add(midiFile);
            midiFilenames->add(file.getFileNameWithoutExtension());
        }
    }

    std::shared_ptr<int> fileCounter{ new int }, trackCounter{ new int };
    *fileCounter = 0;
    *trackCounter = 0;

    this->processor.getUndoManager().beginNewTransaction("'Import MIDI file'");
    loadMidiWizard(fileCounter, trackCounter, midiFiles, midiFilenames);
}

void Project::loadMidiWizard(std::shared_ptr<int> fileCounter, std::shared_ptr<int> trackCounter, 
    std::shared_ptr<OwnedArray<MidiFile>> midiFiles, std::shared_ptr<StringArray> midiFilenames)
{
    if (*fileCounter >= midiFiles->size())
        return;

    auto* midiFile = (*midiFiles)[*fileCounter];
    auto tickLen = 1.0f / (midiFile->getTimeFormat() & 0x7fff);

    if (*trackCounter < midiFile->getNumTracks())
    {
        auto* seq = midiFile->getTrack(*trackCounter);
        if (seq->getNumEvents() > 0)
        {
            auto* window = new AlertWindow("Load track to..",
                "Choose destination for track (" + String(seq->getNumEvents()) + " events)",
                AlertWindow::AlertIconType::NoIcon);
            window->addComboBox("trackbox", {});
            auto* combobox = window->getComboBoxComponent("trackbox");
            combobox->addItem("New...", -1);
            int j = 1;
            for (auto* t : noteTracks)
                combobox->addItem(String(j++) + ": " + t->getTrackName(), t->getTrackId());
            combobox->setSelectedId(-1);

            window->addButton(TRANS("Import"), 1, KeyPress(KeyPress::returnKey));
            window->addButton(TRANS("Skip"), 2);
            window->addButton(TRANS("Cancel"), 0, KeyPress(KeyPress::escapeKey));

            window->enterModalState(true,
                ModalCallbackFunction::create([window, combobox, midiFile, seqc = *seq, fileCounter, trackCounter, midiFiles, midiFilenames, tickLen, this](int r) {
                if (r == 1)
                {
                    IdGenerator::Id id;
                    if (combobox->getSelectedId() == -1)
                    {
                        int c = 1;
                        for (auto i = 0; i < seqc.getNumEvents(); i++)
                        {
                            auto* evt = seqc.getEventPointer(i);
                            if (evt->message.isNoteOn())
                            {
                                c = evt->message.getChannel();
                                break;
                            }
                        }
                        auto* action = new NoteTrackInsertAction(*this, ValueTree(),
                            (*midiFilenames)[*fileCounter] + "-" + String(*trackCounter), jmax(1, c));
                        getUndoManager().perform(action);
                        id = action->getTrackId();
                    }
                    else
                        id = combobox->getSelectedId();

                    if (auto* track = findTrackById<NoteTrack>(id))
                    {
                        Array<Note> seqNotes;
                        for (auto i = 0; i < seqc.getNumEvents(); i++)
                        {
                            auto* evt = seqc.getEventPointer(i);
                            DBG(evt->message.getDescription());
                            if (evt->message.isNoteOn())
                                seqNotes.add(Note(track,
                                    evt->message.getNoteNumber(),
                                    evt->message.getTimeStamp() * tickLen,
                                    (evt->noteOffObject->message.getTimeStamp() - evt->message.getTimeStamp()) * tickLen,
                                    evt->message.getFloatVelocity()));
                        }
                        track->insertGroup(seqNotes, true);
                    }
                }
                (*trackCounter)++;
                if (*trackCounter > midiFile->getNumTracks() + 1)
                {
                    (*fileCounter)++;
                    *trackCounter = 0;
                }
                if (r != 0)
                    loadMidiWizard(fileCounter, trackCounter, midiFiles, midiFilenames);

            }),
                true);
        }
    }
    else if (*trackCounter == midiFile->getNumTracks())
    {
        MidiMessageSequence markers;
        midiFile->findAllTempoEvents(markers);
        if (markers.getNumEvents() > 0)
        {
            auto* window = new AlertWindow("Load track to..",
                "What to do with the tempo track?",
                AlertWindow::AlertIconType::QuestionIcon);

            window->addButton(TRANS("Skip"), 0, KeyPress(KeyPress::returnKey));
            window->addButton(TRANS("Overwrite"), 1);
            window->addButton(TRANS("Merge"), 2);

            window->enterModalState(true,
                ModalCallbackFunction::create([window, tickLen, markers, midiFile, fileCounter, trackCounter, midiFiles, midiFilenames, this](int r) {
                    if (r == 1 || r == 2)
                    {
                        Array<TempoMarkerEvent> newEvts;
                        for (auto i = 0; i < markers.getNumEvents(); i++)
                        {
                            auto* evt = markers.getEventPointer(i);
                            if (evt->message.isTempoMetaEvent())
                                newEvts.add(TempoMarkerEvent(tempoMarkerTrack.get(),
                                    evt->message.getTimeStamp() * tickLen, 60.0f / evt->message.getTempoSecondsPerQuarterNote()));
                        }
                        if (r == 1)
                        {
                            Array<TempoMarkerEvent> evts;
                            for (auto* evt : *tempoMarkerTrack)
                                evts.add(*dynamic_cast<TempoMarkerEvent*>(evt));
                            tempoMarkerTrack->removeGroup(evts, true);
                        }
                        tempoMarkerTrack->insertGroup(newEvts, true);
                    }
                    (*trackCounter)++;
                    if (*trackCounter > midiFile->getNumTracks() + 1)
                    {
                        (*fileCounter)++;
                        *trackCounter = 0;
                    }
                    loadMidiWizard(fileCounter, trackCounter, midiFiles, midiFilenames);
                    }),
                true);
        }
    }
    else if (*trackCounter == midiFile->getNumTracks() + 1)
    {
        MidiMessageSequence sigs;
        midiFile->findAllTimeSigEvents(sigs);
        if (sigs.getNumEvents() > 0)
        {
            auto* window = new AlertWindow("Load track to..",
                "What to do with the time signature track?",
                AlertWindow::AlertIconType::QuestionIcon);

            window->addButton(TRANS("Skip"), 0, KeyPress(KeyPress::returnKey));
            window->addButton(TRANS("Overwrite"), 1);
            window->addButton(TRANS("Merge"), 2);

            window->enterModalState(true,
                ModalCallbackFunction::create([window, tickLen, sigs, midiFile, fileCounter, trackCounter, midiFiles, midiFilenames, this](int r) {
                    if (r == 1 || r == 2)
                    {
                        Array<TimeSignatureEvent> newEvts;
                        for (auto i = 0; i < sigs.getNumEvents(); i++)
                        {
                            auto* evt = sigs.getEventPointer(i);
                            if (evt->message.isTimeSignatureMetaEvent())
                            {
                                int numerator, denominator;
                                evt->message.getTimeSignatureInfo(numerator, denominator);
                                newEvts.add(TimeSignatureEvent(timeSignatureTrack.get(),
                                    evt->message.getTimeStamp() * tickLen, numerator, denominator));
                            }
                        }
                        if (r == 1)
                        {
                            Array<TimeSignatureEvent> evts;
                            for (auto* evt : *timeSignatureTrack)
                                evts.add(*dynamic_cast<TimeSignatureEvent*>(evt));
                            timeSignatureTrack->removeGroup(evts, true);
                        }
                        timeSignatureTrack->insertGroup(newEvts, true);
                    }
                    (*trackCounter)++;
                    if (*trackCounter > midiFile->getNumTracks() + 1)
                    {
                        (*fileCounter)++;
                        *trackCounter = 0;
                    }
                    loadMidiWizard(fileCounter, trackCounter, midiFiles, midiFilenames);
                    }),
                true);
        }
    }
}
