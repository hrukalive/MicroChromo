/*
  ==============================================================================

    MainEditor.cpp
    Created: 8 Apr 2020 9:50:26am
    Author:  bowen

  ==============================================================================
*/

#include "MainEditor.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MainEditor::TextButtonDropTarget::TextButtonDropTarget(String text, MainEditor& owner) :
    TextButton(text), _owner(owner) {}

bool MainEditor::TextButtonDropTarget::isInterestedInFileDrag(const StringArray& files)
{
    return true;
}

void MainEditor::TextButtonDropTarget::filesDropped(const StringArray& files, int x, int y)
{
    _owner.itemDroppedCallback(files);
}

//==============================================================================
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
    synthButton->addMouseListener(this, true);
    synthLabel.reset(new Label("SynthLabel", "<empty>"));
    addAndMakeVisible(synthLabel.get());

    effectButton.reset(new TextButton("Effect"));
    addAndMakeVisible(effectButton.get());
    effectButton->addMouseListener(this, true);
    effectLabel.reset(new Label("EffectLabel", "<empty>"));
    addAndMakeVisible(effectLabel.get());

    dragButton.reset(new TextButton("Drag MIDI"));
    addAndMakeVisible(dragButton.get());
    dragButton->addMouseListener(this, true);

    dropButton.reset(new TextButtonDropTarget("Drop MIDI Here", *this));
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
    numParameterSlot->setText(String(p.getParameterSlotNumber()));
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
    numInstancesBox->setSelectedId(processor.getNumInstances());
    numInstancesBox->onChange = [&]() {
        if (!ignoreInitialChange1)
        {
            synthBundle->closeAllWindows();
            psBundle->closeAllWindows();
            processor.adjustInstanceNumber(numInstancesBox->getSelectedId());
        }
        ignoreInitialChange1 = false;
    };
    numInstancesLabel.reset(new Label());
    addAndMakeVisible(numInstancesLabel.get());
    numInstancesLabel->setText("# Instance", dontSendNotification);

    midiChannelComboBox.reset(new ComboBox());
    addAndMakeVisible(midiChannelComboBox.get());
    for (int i = 1; i <= 16; i++)
        midiChannelComboBox->addItem(String(i), i);
    midiChannelComboBox->setSelectedId(processor.getMidiChannel());
    midiChannelComboBox->onChange = [&]() {
        if (!ignoreInitialChange2)
        {
            processor.updateMidiChannel(midiChannelComboBox->getSelectedId());
        }
        ignoreInitialChange2 = false;
    };
    midiChannelLabel.reset(new Label());
    addAndMakeVisible(midiChannelLabel.get());
    midiChannelLabel->setText("MIDI Channel", dontSendNotification);

    synthBundle->addChangeListener(this);
    psBundle->addChangeListener(this);

    changeListenerCallback(synthBundle.get());
    changeListenerCallback(psBundle.get());

    setResizable(true, false);
    setSize(400, 230);
}

MainEditor::~MainEditor()
{
    synthButton->removeMouseListener(this);
    effectButton->removeMouseListener(this);
    dragButton->removeMouseListener(this);
}

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

void MainEditor::itemDroppedCallback(const StringArray& files)
{
    DBG("Something dropped");
}

void MainEditor::mouseDown(const MouseEvent& e)
{
    if (e.eventComponent == synthButton.get())
    {
        floatMenu = synthBundle->getPluginPopupMenu(_parent.getPluginSortMethod(), processor.getSynthKnownPluginList());
        PopupMenu::Options options;
        floatMenu->showMenuAsync(options.withTargetComponent(synthButton.get()).withMaximumNumColumns(1),
            ModalCallbackFunction::create([this](int r) { bundlePopupMenuSelected(r, true); }));
    }
    else if (e.eventComponent == effectButton.get())
    {
        floatMenu = psBundle->getPluginPopupMenu(_parent.getPluginSortMethod(), processor.getPsKnownPluginList());
        PopupMenu::Options options;
        floatMenu->showMenuAsync(options.withTargetComponent(effectButton.get()).withMaximumNumColumns(1),
            ModalCallbackFunction::create([this](int r) { bundlePopupMenuSelected(r, false); }));
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

            auto& notesMidiSeq = processor.getNoteMidiSequence();
            auto& ccMidiSeq = processor.getCcMidiSequence();

            for (int i = 0; i < notesMidiSeq.size(); i++)
            {
                if (notesMidiSeq[i]->getNumEvents() == 0)
                    continue;

                MidiMessageSequence copy;
                for (auto& evt : *notesMidiSeq[i])
                    copy.addEvent(MidiMessage(evt->message).withTimeStamp(evt->message.getTimeStamp() * 48000));
                for (auto& evt : *ccMidiSeq[i])
                    copy.addEvent(MidiMessage(evt->message).withTimeStamp(evt->message.getTimeStamp() * 48000));
                copy.sort();
                copy.updateMatchedPairs();

                MidiFile file;
                file.addTrack(copy);

                std::shared_ptr<TemporaryFile> outf = std::make_shared<TemporaryFile>(".mid");
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

void MainEditor::timerCallback()
{
    canDrop = true;
    stopTimer();
}

void MainEditor::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void MainEditor::resized()
{
    auto b = getLocalBounds();
    b.reduce(10, 10);
    auto halfWidth = b.proportionOfWidth(0.48);
    auto halfWidthSpace = b.proportionOfWidth(0.04);
    auto spaceHeightSmall = jmax(1.0f, (b.getHeight() - 200) / 30.0f * 4);
    auto spaceHeightLarge = jmax(1.0f, (b.getHeight() - 200) / 30.0f * 9);

    auto tmp = b.removeFromBottom(30);
    dragButton->setBounds(tmp.removeFromLeft(halfWidth));
    tmp.removeFromLeft(halfWidthSpace);
    dropButton->setBounds(tmp);

    auto leftPanel = b.removeFromLeft(b.proportionOfWidth(0.3));
    auto rightPanel = b.withTrimmedLeft(10);

    numInstancesLabel->setBounds(leftPanel.removeFromTop(30));
    leftPanel.removeFromTop(spaceHeightSmall);
    numParameterLabel->setBounds(leftPanel.removeFromTop(30));
    leftPanel.removeFromTop(spaceHeightSmall);
    midiChannelLabel->setBounds(leftPanel.removeFromTop(30));
    leftPanel.removeFromTop(spaceHeightLarge);

    synthButton->setBounds(leftPanel.removeFromTop(40));
    leftPanel.removeFromTop(spaceHeightSmall);
    effectButton->setBounds(leftPanel.removeFromTop(40));

    numInstancesBox->setBounds(rightPanel.removeFromTop(30));
    rightPanel.removeFromTop(spaceHeightSmall);
    numParameterSlot->setBounds(rightPanel.removeFromTop(30));
    rightPanel.removeFromTop(spaceHeightSmall);
    midiChannelComboBox->setBounds(rightPanel.removeFromTop(30));
    rightPanel.removeFromTop(spaceHeightLarge);

    synthLabel->setBounds(rightPanel.removeFromTop(40));
    rightPanel.removeFromTop(spaceHeightSmall);
    effectLabel->setBounds(rightPanel.removeFromTop(40));
}
