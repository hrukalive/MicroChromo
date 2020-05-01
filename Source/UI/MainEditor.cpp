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

#include "MainEditor.h"

#include "PluginProcessor.h"
#include "PluginBundle.h"
#include "PluginEditor.h"

//__________________________________________________________________________
//                                                                          |\
// TextButtonDropTarget                                                     | |
//__________________________________________________________________________| |
//___________________________________________________________________________\|

MainEditor::TextButtonDropTarget::TextButtonDropTarget(String text, MainEditor& owner) :
    TextButton(text), _owner(owner) {}

bool MainEditor::TextButtonDropTarget::isInterestedInFileDrag(const StringArray& files)
{
    for (auto& f : files)
    {
        if (!f.endsWith(".mid") && !f.endsWith(".midi"))
            return false;
    }
    return true;
}

void MainEditor::TextButtonDropTarget::filesDropped(const StringArray& files, int x, int y)
{
    Array<File> fs;
    for (auto filepath : files)
        fs.add(File(filepath));
    _owner.itemDroppedCallback(fs);
}

//__________________________________________________________________________
//                                                                          |\
// MainEditor                                                               | |
//__________________________________________________________________________| |
//___________________________________________________________________________\|

MainEditor::MainEditor(MicroChromoAudioProcessor& p, MicroChromoAudioProcessorEditor& parent)
    : AudioProcessorEditor(&p),
    _parent(parent),
    processor(p),
    appProperties(p.getApplicationProperties()),
    synthBundle(p.getSynthBundlePtr()),
    psBundle(p.getPSBundlePtr())
{
    bundlePopupMenuSelected = [this](int r, bool isSynth) {
        auto bundle = isSynth ? this->synthBundle : this->psBundle;
        switch (r)
        {
        case SLOT_MENU_LOAD_EMPTY_PLUGIN:
        {
            bundle->closeAllWindows();
            processor.addPlugin(bundle->getEmptyPluginDescription(), isSynth);
            break;
        }
        case SLOT_MENU_LOAD_DEFAULT_PLUGIN:
        {
            bundle->closeAllWindows();
            processor.addPlugin(bundle->getDefaultPluginDescription(), isSynth);
            break;
        }
        default:
        {
            if (r >= pluginMenuIdBase)
            {
                bundle->closeAllWindows();
                auto types = (isSynth ? processor.getSynthKnownPluginList() : processor.getPsKnownPluginList()).getTypes();
                int result = KnownPluginList::getIndexChosenByMenu(types, r);
                auto& desc = types.getReference(result);
                processor.addPlugin(desc, isSynth);
            }
        }
        }
    };

    synthButton.reset(new TextButton("Synth"));
    addAndMakeVisible(synthButton.get());
    synthButton->onClick = [&]() {
        floatMenu = synthBundle->getPluginPopupMenu(_parent.getPluginSortMethod(), processor.getSynthKnownPluginList());
        PopupMenu::Options options;
        floatMenu->showMenuAsync(options.withTargetComponent(synthButton.get()).withMaximumNumColumns(1),
            ModalCallbackFunction::create([this](int r) { bundlePopupMenuSelected(r, true); }));
    };
    synthLabel.reset(new Label("SynthLabel", "<empty>"));
    addAndMakeVisible(synthLabel.get());

    effectButton.reset(new TextButton("Effect"));
    addAndMakeVisible(effectButton.get());
    effectButton->onClick = [&]() {
        floatMenu = psBundle->getPluginPopupMenu(_parent.getPluginSortMethod(), processor.getPsKnownPluginList());
        PopupMenu::Options options;
        floatMenu->showMenuAsync(options.withTargetComponent(effectButton.get()).withMaximumNumColumns(1),
            ModalCallbackFunction::create([this](int r) { bundlePopupMenuSelected(r, false); }));
    };
    effectLabel.reset(new Label("EffectLabel", "<empty>"));
    addAndMakeVisible(effectLabel.get());

    dragButton.reset(new TextButton("Drag MIDI"));
    addAndMakeVisible(dragButton.get());
    dragButton->addMouseListener(this, true);

    dropButton.reset(new TextButtonDropTarget("Drop MIDI Here", *this));
    dropButton->onClick = [&]() {
        FileChooser chooser("Import MIDI file",
            lastOpenedLocation == "" ? File::getSpecialLocation(File::userHomeDirectory) : lastOpenedLocation,
            "*.mid; *.midi");
        if (chooser.browseForMultipleFilesToOpen())
        {
            lastOpenedLocation = chooser.getResults()[0].getParentDirectory().getFullPathName();
            itemDroppedCallback(chooser.getResults());
        }
    };
    addAndMakeVisible(dropButton.get());

    numParameterSlot.reset(new TextEditor());
    addAndMakeVisible(numParameterSlot.get());
    numParameterSlot->setMultiLine(false);
    numParameterSlot->setReturnKeyStartsNewLine(false);
    numParameterSlot->setReadOnly(false);
    numParameterSlot->setInputRestrictions(4, "0123456789");
    numParameterSlot->setScrollbarsShown(false);
    numParameterSlot->setCaretVisible(true);
    numParameterSlot->setPopupMenuEnabled(false);
    numParameterSlot->setText(String(processor.getParameterSlotNumber()));
    numParameterSlot->onTextChange = [&]()
    {
        appProperties.getUserSettings()->setValue("parameterSlotNumber", numParameterSlot->getText().getIntValue());
        appProperties.saveIfNeeded();
    };
    numParameterLabel.reset(new Label());
    addAndMakeVisible(numParameterLabel.get());
    numParameterLabel->setText("# Parameter Slots", dontSendNotification);

    numInstancesBox.reset(new ComboBox());
    addAndMakeVisible(numInstancesBox.get());
    for (int i = 1; i <= MicroChromoAudioProcessor::MAX_INSTANCES; i++)
        numInstancesBox->addItem(String(i), i);
    numInstancesBox->setSelectedId(processor.getNumInstances(), dontSendNotification);
    numInstancesBox->onChange = [&]() {
        synthBundle->closeAllWindows();
        psBundle->closeAllWindows();
        processor.adjustInstanceNumber(numInstancesBox->getSelectedId());
    };
    numInstancesLabel.reset(new Label());
    addAndMakeVisible(numInstancesLabel.get());
    numInstancesLabel->setText("# Instance", dontSendNotification);

    midiChannelComboBox.reset(new ComboBox());
    addAndMakeVisible(midiChannelComboBox.get());
    midiChannelComboBox->addItem("Omni", 1);
    for (int i = 1; i <= 16; i++)
        midiChannelComboBox->addItem(String(i), i + 1);
    midiChannelComboBox->setSelectedId(processor.getMidiChannel() + 1, dontSendNotification);
    midiChannelComboBox->onChange = [&]() {
        processor.updateMidiChannel(midiChannelComboBox->getSelectedId() - 1);
    };
    midiChannelLabel.reset(new Label());
    addAndMakeVisible(midiChannelLabel.get());
    midiChannelLabel->setText("MIDI Channel", dontSendNotification);

    pbRangeTextbox.reset(new TextEditor());
    addAndMakeVisible(pbRangeTextbox.get());
    pbRangeTextbox->setMultiLine(false);
    pbRangeTextbox->setReturnKeyStartsNewLine(false);
    pbRangeTextbox->setReadOnly(false);
    pbRangeTextbox->setInputRestrictions(4, "0123456789.");
    pbRangeTextbox->setScrollbarsShown(false);
    pbRangeTextbox->setCaretVisible(true);
    pbRangeTextbox->setPopupMenuEnabled(false);
    pbRangeTextbox->setText(String(processor.getPitchbendRange(), 1));
    pbRangeTextbox->onTextChange = [&]() {
        processor.setPitchbendRange(jlimit(0.0f, 12.0f, pbRangeTextbox->getText().getFloatValue()));
    };
    pbRangeLabel.reset(new Label());
    addAndMakeVisible(pbRangeLabel.get());
    pbRangeLabel->setText("Pitchbend Range", dontSendNotification);

    tailLenTextbox.reset(new TextEditor());
    addAndMakeVisible(tailLenTextbox.get());
    tailLenTextbox->setMultiLine(false);
    tailLenTextbox->setReturnKeyStartsNewLine(false);
    tailLenTextbox->setReadOnly(false);
    tailLenTextbox->setInputRestrictions(4, "0123456789.");
    tailLenTextbox->setScrollbarsShown(false);
    tailLenTextbox->setCaretVisible(true);
    tailLenTextbox->setPopupMenuEnabled(false);
    tailLenTextbox->setText(String(processor.getTailLength(), 1));
    tailLenTextbox->onTextChange = [&]() {
        processor.setTailLength(jlimit(0.0f, FLT_MAX, tailLenTextbox->getText().getFloatValue()));
    };
    tailLenLabel.reset(new Label());
    addAndMakeVisible(tailLenLabel.get());
    tailLenLabel->setText("Tail Length (s)", dontSendNotification);

    synthBundle->addChangeListener(this);
    psBundle->addChangeListener(this);

    processor.getProject().addListener(this);

    changeListenerCallback(synthBundle.get());
    changeListenerCallback(psBundle.get());

    setResizable(true, false);
    setSize(400, 480);
}

MainEditor::~MainEditor()
{
    processor.getProject().removeListener(this);

    synthBundle->removeChangeListener(this);
    psBundle->removeChangeListener(this);

    synthButton->removeMouseListener(this);
    effectButton->removeMouseListener(this);
    dragButton->removeMouseListener(this);
}

//===------------------------------------------------------------------===//
// Callbacks
//===------------------------------------------------------------------===//
void MainEditor::changeListenerCallback(ChangeBroadcaster* changed)
{
    if (changed == synthBundle.get())
    {
        if (synthBundle->isLoading())
            synthLabel->setText("Loading...", NotificationType::dontSendNotification);
        else if (synthBundle->isLoaded())
            synthLabel->setText(synthBundle->getName(), NotificationType::dontSendNotification);
    }
    else if (changed == psBundle.get())
    {
        if (psBundle->isLoading())
            effectLabel->setText("Loading...", NotificationType::dontSendNotification);
        else if (psBundle->isLoaded())
            effectLabel->setText(psBundle->getName(), NotificationType::dontSendNotification);
    }
}

void MainEditor::timerCallback()
{
    canDrop = true;
    stopTimer();
}

void MainEditor::itemDroppedCallback(const Array<File>& files)
{
    processor.getProject().loadMidiFile(files);
}

//===------------------------------------------------------------------===//
// Mouse Listeners
//===------------------------------------------------------------------===//
void MainEditor::mouseDoubleClick(const MouseEvent& e)
{
    if (e.eventComponent == dragButton.get())
    {
        exportMidiDialog();
    }
}

void MainEditor::mouseDrag(const MouseEvent& e)
{
    if (e.eventComponent == dragButton.get() && canDrop)
    {
        DragAndDropContainer* dragContainer = DragAndDropContainer::findParentDragContainerFor(dragButton.get());
        if (!dragContainer->isDragAndDropActive())
        {
            Array<std::shared_ptr<TemporaryFile>> files;
            StringArray filenames;
            auto tmpFilename = "temp_" + String::toHexString(Random::getSystemRandom().nextInt());

            int i = 1;
            for (auto& file : processor.getProject().exportMidiFiles())
            {
                std::shared_ptr<TemporaryFile> outf = std::make_shared<TemporaryFile>(File(),
                    File::getSpecialLocation(File::tempDirectory).getNonexistentChildFile(tmpFilename + "_" + String(i++), ".mid"));
                if (std::unique_ptr<FileOutputStream> p_os{ outf->getFile().createOutputStream() })
                {
                    file.writeTo(*p_os, 1);
                    files.add(outf);
                    filenames.add(outf->getFile().getFullPathName());
                }
            }

            canDrop = false;
            startTimer(1000);

            DragAndDropContainer::performExternalDragDropOfFiles(filenames, false, nullptr,
                [=]() {
                    for (auto& outf : files)
                        outf->deleteTemporaryFile();
                    DBG("Dropped");
                });
        }
    }
}

//===------------------------------------------------------------------===//
// Project Listener
//===------------------------------------------------------------------===//
void MainEditor::onReloadProjectContent(const Array<MidiTrack*>& tracks)
{
    numParameterSlot->setText(String(processor.getParameterSlotNumber()), sendNotificationSync);
    numInstancesBox->setSelectedId(processor.getNumInstances(), dontSendNotification);
    midiChannelComboBox->setSelectedId(processor.getMidiChannel() + 1, dontSendNotification);
}

//===------------------------------------------------------------------===//
// Helpers
//===------------------------------------------------------------------===//
void MainEditor::exportMidiDialog()
{
    FileChooser chooser("Export MIDI file to...",
        lastSavedLocation == "" ? File::getSpecialLocation(File::userDocumentsDirectory) : lastSavedLocation,
        "*.mid; *.midi");
    if (chooser.browseForDirectory())
    {
        lastSavedLocation = chooser.getResult().getFullPathName();

        auto tmpFilename = "temp_" + String::toHexString(Random::getSystemRandom().nextInt());
        int i = 1;
        for (auto& file : processor.getProject().exportMidiFiles())
        {
            File outf = File(chooser.getResult().getFullPathName()).getNonexistentChildFile(tmpFilename + "_" + String(i++), ".mid");
            if (std::unique_ptr<FileOutputStream> p_os{ outf.createOutputStream() })
                file.writeTo(*p_os, 1);
        }
    }
}

//===------------------------------------------------------------------===//
// Components
//===------------------------------------------------------------------===//
void MainEditor::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void MainEditor::resized()
{
    auto b = getLocalBounds();
    b.reduce(10, 10);

    auto slice = b.proportionOfHeight(1 / 27.0f);

    auto tmp = b.removeFromBottom(4 * slice).reduced(0, 2);
    dragButton->setBounds(tmp.removeFromLeft(tmp.proportionOfWidth(0.5)).reduced(2, 0));
    dropButton->setBounds(tmp.reduced(2, 0));

    auto leftPanel = b.removeFromLeft(b.proportionOfWidth(0.3));
    auto rightPanel = b.withTrimmedLeft(10);

    numInstancesLabel->setBounds(leftPanel.removeFromTop(3 * slice).reduced(0, 4));
    numParameterLabel->setBounds(leftPanel.removeFromTop(3 * slice).reduced(0, 4));
    midiChannelLabel->setBounds(leftPanel.removeFromTop(3 * slice).reduced(0, 4));
    pbRangeLabel->setBounds(leftPanel.removeFromTop(3 * slice).reduced(0, 4));
    tailLenLabel->setBounds(leftPanel.removeFromTop(3 * slice).reduced(0, 4));

    synthButton->setBounds(leftPanel.removeFromTop(4 * slice).reduced(0, 4));
    effectButton->setBounds(leftPanel.removeFromTop(4 * slice).reduced(0, 4));

    numInstancesBox->setBounds(rightPanel.removeFromTop(3 * slice).reduced(0, 4));
    numParameterSlot->setBounds(rightPanel.removeFromTop(3 * slice).reduced(0, 4));
    midiChannelComboBox->setBounds(rightPanel.removeFromTop(3 * slice).reduced(0, 4));
    pbRangeTextbox->setBounds(rightPanel.removeFromTop(3 * slice).reduced(0, 4));
    tailLenTextbox->setBounds(rightPanel.removeFromTop(3 * slice).reduced(0, 4));

    synthLabel->setBounds(rightPanel.removeFromTop(4 * slice).reduced(0, 4));
    effectLabel->setBounds(rightPanel.removeFromTop(4 * slice).reduced(0, 4));
}
