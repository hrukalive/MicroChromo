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

#include "ColorEditor.h"

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Note.h"

//__________________________________________________________________________
//                                                                          |\
// ColorChooserComponent                                                    | |
//__________________________________________________________________________| |
//___________________________________________________________________________\|

ColorEditor::ColorChooserComponent::ColorChooserComponent(
    ColorEditor& parent,
    const int row, const int col, const Colour& color) :
    _parent(parent)
{
    addAndMakeVisible(selector);

    rowId = row;
    columnId = col;
    selector.setCurrentColour(color);

    btn.onClick = [this]() {
        _parent.colorChooserClosed(rowId, columnId, selector.getCurrentColour());
        this->getTopLevelComponent()->exitModalState(0);
    };
    addAndMakeVisible(btn);

    setSize(300, 400);
}

//===------------------------------------------------------------------===//
// Components
//===------------------------------------------------------------------===//
void ColorEditor::ColorChooserComponent::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void ColorEditor::ColorChooserComponent::resized()
{
    auto b = getLocalBounds();
    b.reduce(10, 10);
    btn.setBounds(b.removeFromBottom(24));
    b.removeFromBottom(4);
    selector.setBounds(b);
}

//__________________________________________________________________________
//                                                                          |\
// DefaultKeyChooser                                                        | |
//__________________________________________________________________________| |
//___________________________________________________________________________\|

ColorEditor::DefaultKeyChooser::DefaultKeyChooser(ColorEditor& parent, const int row, const int col,
    const std::unordered_set<int>& used, const std::unordered_set<int>& current) :
    _parent(parent)
{
    noteList.setModel(this);
    addAndMakeVisible(noteList);
    noteList.setColour(ListBox::backgroundColourId, Colours::black.withAlpha(0.02f));
    noteList.setColour(ListBox::outlineColourId, Colours::black.withAlpha(0.1f));
    noteList.setOutlineThickness(1);

    rowId = row;
    columnId = col;

    usedSet = used;
    for (int i : current)
    {
        currentSet.insert(i);
        selectedSet.insert(i);
    }

    addAndMakeVisible(okBtn);
    okBtn.onClick = [&]()
    {
        _parent.defaultKeyChooserClosed(rowId, columnId, selectedSet);
        this->getTopLevelComponent()->exitModalState(0);
    };

    setSize(300, 400);
}

//===------------------------------------------------------------------===//
// ListBoxModel
//===------------------------------------------------------------------===//
void ColorEditor::DefaultKeyChooser::paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool /*rowIsSelected*/)
{
    if (selectedSet.contains(rowNumber))
        g.fillAll(findColour(TextEditor::highlightColourId));

    if (!usedSet.contains(rowNumber) || currentSet.contains(rowNumber))
        g.setColour(findColour(ListBox::textColourId));
    else
        g.setColour(findColour(ListBox::textColourId).darker());
    Font f(height * 0.7f);
    f.setHorizontalScale(0.9f);
    g.setFont(f);

    g.drawText(MidiMessage::getMidiNoteName(rowNumber, true, false, 4),
        4, 0, width - 6, height,
        Justification::centredLeft, true);
}

void ColorEditor::DefaultKeyChooser::listBoxItemClicked(int row, const MouseEvent&)
{
    if (usedSet.contains(row) && !currentSet.contains(row))
        return;
    if (selectedSet.contains(row))
        selectedSet.erase(row);
    else
        selectedSet.insert(row);
    noteList.repaintRow(row);
}

//===------------------------------------------------------------------===//
// Components
//===------------------------------------------------------------------===//
void ColorEditor::DefaultKeyChooser::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void ColorEditor::DefaultKeyChooser::resized()
{
    auto b = getLocalBounds();
    b.reduce(10, 10);
    okBtn.setBounds(b.removeFromBottom(40));
    b.removeFromBottom(10);
    noteList.setBounds(b);
}

//__________________________________________________________________________
//                                                                          |\
// ColorEditor                                                              | |
//__________________________________________________________________________| |
//___________________________________________________________________________\|

ColorEditor::ColorEditor(MicroChromoAudioProcessorEditor& editor) :
    owner(editor), processor(editor.getProcessor()),
    appProperties(processor.getApplicationProperties()),
    project(editor.getProcessor().getProject())
{
    loadColorPreset();

    int columnId = 1;
    table.getHeader().addColumn("Name", columnId++, 120, 100, 200, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("Color", columnId++, 50, 50, 100, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("Value", columnId++, 50, 50, 100, TableHeaderComponent::notSortable | TableHeaderComponent::sortedForwards);
    table.getHeader().addColumn("Default Key", columnId++, 120, 100, 200, TableHeaderComponent::notSortable);

    table.setColour(ListBox::outlineColourId, Colours::grey);
    table.setOutlineThickness(1);
    table.setMultipleSelectionEnabled(false);
    table.getHeader().setSortColumnId(3, true);

    addAndMakeVisible(table);

    addAndMakeVisible(addEntryBtn);
    addEntryBtn.onClick = [&]() {
        auto* colorMap = project.getPitchColorMap();
        int i = 1;
        for (; colorMap->hasNamedUsed("New" + String(i)); i++);
        processor.getUndoManager().beginNewTransaction("'Add a pitch color entry'");
        colorMap->insert(PitchColorMapEntry(colorMap, "New" + String(i), 0), true);
    };

    addAndMakeVisible(removeEntryBtn);
    removeEntryBtn.onClick = [&]() {
        auto* entry = (*project.getPitchColorMap())[table.getSelectedRow(0)];
        String name = entry->getName();
        processor.getUndoManager().beginNewTransaction("'Remove selected entry'");
        project.getPitchColorMap()->remove(*entry, true);
    };

    addAndMakeVisible(presetComboBox);
    presetComboBox.onChange = [&]() {
        if (presetComboBox.getSelectedItemIndex() > -1)
        {
            auto* colorMap = project.getPitchColorMap();
            auto* m = colorMapPresets[presetComboBox.getSelectedItemIndex()];
            processor.setSelectedColorPresetName(m->getName());
            processor.getUndoManager().beginNewTransaction("'Apply color map preset [" + m->getName() + "]'");
            colorMap->removeGroup(colorMap->getAllEntries(), true);
            colorMap->insertGroup(m->getAllEntries(), true);

            checkModified();
        }
    };

    addAndMakeVisible(savePresetBtn);
    savePresetBtn.onClick = [&]() {
        AlertWindow* window = new AlertWindow("Add Preset", "Enter name", AlertWindow::AlertIconType::NoIcon);
        window->addTextEditor("nameEditor", "", "", false);
        window->addButton("OK", 1, KeyPress(KeyPress::returnKey, 0, 0));
        window->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey, 0, 0));
        window->enterModalState(true, ModalCallbackFunction::create([window, this](int r) {
            if (r)
            {
                auto name = window->getTextEditorContents("nameEditor");
                if (name == "---INIT---")
                {
                    AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, "Error", "Name entered is reserved.");
                    return;
                }
                bool overwrite = false;
                for (int i = 0; i < colorMapPresets.size(); i++)
                    if (colorMapPresets[i]->getName() == name)
                    {
                        colorMapPresets.set(i, new PitchColorMap(nullptr, name, project.getPitchColorMap()->getAllEntries()));
                        overwrite = true;
                    }
                if (!overwrite)
                    colorMapPresets.add(new PitchColorMap(nullptr, name, project.getPitchColorMap()->getAllEntries()));
                this->updatePresetComboBox(name);
                this->saveColorMapPresets();
            }
            }), true);
    };

    addAndMakeVisible(deletePresetBtn);
    deletePresetBtn.onClick = [&]() {
        colorMapPresets.remove(presetComboBox.getSelectedItemIndex());
        this->updatePresetComboBox("---INIT---");
        this->saveColorMapPresets();
    };

    addAndMakeVisible(renamePresetBtn);
    renamePresetBtn.onClick = [&]() {
        if (presetComboBox.getSelectedItemIndex() > -1)
        {
            auto* m = colorMapPresets[presetComboBox.getSelectedItemIndex()];
            AlertWindow* window = new AlertWindow("Change Preset", "Enter new name", AlertWindow::AlertIconType::NoIcon);
            window->addTextEditor("nameEditor", m->getName());
            window->addButton("OK", 1, KeyPress(KeyPress::returnKey, 0, 0));
            window->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey, 0, 0));
            window->enterModalState(true, ModalCallbackFunction::create([window, m, this](int r) {
                if (r)
                {
                    auto name = window->getTextEditorContents("nameEditor");
                    if (name == "---INIT---")
                    {
                        AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, "Error", "Name entered is reserved.");
                        return;
                    }
                    if (name != m->getName())
                    {
                        for (int i = 0; i < colorMapPresets.size(); i++)
                            if (colorMapPresets[i]->getName() == name)
                                colorMapPresets.remove(i);
                        m->setName(name, false);
                        colorMapPresets.sort(PitchColorMapNameComparator(), true);
                        this->updatePresetComboBox(name);
                        this->saveColorMapPresets();
                    }
                }
                }), true);
        }
    };

    loadColorPreset();
    updatePresetComboBox(processor.getSelectedColorPresetName());

    project.addListener(this);

    setSize(365, 300);
}

ColorEditor::~ColorEditor()
{
    project.removeListener(this);
}

//===------------------------------------------------------------------===//
// TableListModel
//===------------------------------------------------------------------===//
int ColorEditor::getNumRows()
{
    return project.getPitchColorMap()->size();
}

void ColorEditor::paintRowBackground(Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected)
{
    auto alternateColour = getLookAndFeel().findColour(ListBox::backgroundColourId)
        .interpolatedWith(getLookAndFeel().findColour(ListBox::textColourId), 0.03f);
    if (rowIsSelected)
        g.fillAll(findColour(TextEditor::highlightColourId));
    else if (rowNumber % 2)
        g.fillAll(alternateColour);
}

void ColorEditor::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/)
{
    g.setColour(getLookAndFeel().findColour(ListBox::textColourId));
    g.setFont(font);
    if (columnId == 2)
    {
        g.setColour((*project.getPitchColorMap())[rowNumber]->getColor());
        g.fillRect(4, 4, width - 8, height - 8);
    }
    else if (columnId == 4)
    {
        String ks = (*project.getPitchColorMap())[rowNumber]->defaultKeysToNoteNames();
        g.drawText(ks.isEmpty() ? "None" : ks, 2, 0, width - 4, height, Justification::centredLeft, true);
    }

    g.setColour(getLookAndFeel().findColour(ListBox::backgroundColourId));
    g.fillRect(width - 1, 0, 1, height);
}

Component* ColorEditor::refreshComponentForCell(int rowNumber, int columnId, bool /*isRowSelected*/, Component* existingComponentToUpdate)
{
    if (columnId == 1 || columnId == 3)
    {
        auto* textLabel = static_cast<EditableTextCustomComponent*> (existingComponentToUpdate);
        if (textLabel == nullptr)
            textLabel = new EditableTextCustomComponent(*this);
        textLabel->setRowAndColumn(rowNumber, columnId);
        if ((*project.getPitchColorMap())[rowNumber]->getName() == "0")
            textLabel->setEnabled(false);
        else
            textLabel->setEnabled(true);
        return textLabel;
    }

    jassert(existingComponentToUpdate == nullptr);
    return nullptr;
}

void ColorEditor::cellClicked(int rowNumber, int columnId, const MouseEvent&)
{
    if (columnId == 2)
    {
        DialogWindow::LaunchOptions dialogOption;

        dialogOption.dialogTitle = "Color Selector";
        dialogOption.dialogBackgroundColour = LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId);
        dialogOption.escapeKeyTriggersCloseButton = false;
        dialogOption.useNativeTitleBar = false;
        dialogOption.resizable = true;
        dialogOption.content.setOwned(new ColorChooserComponent(*this, rowNumber, columnId, (*project.getPitchColorMap())[rowNumber]->getColor()));
        dialogOption.launchAsync();
    }
    else if (columnId == 4)
    {
        DialogWindow::LaunchOptions dialogOption;

        dialogOption.dialogTitle = "Default Key Selector";
        dialogOption.dialogBackgroundColour = LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId);
        dialogOption.escapeKeyTriggersCloseButton = false;
        dialogOption.useNativeTitleBar = false;
        dialogOption.resizable = true;
        dialogOption.content.setOwned(new DefaultKeyChooser(*this, rowNumber, columnId,
            project.getPitchColorMap()->getUsedNotes(), (*project.getPitchColorMap())[rowNumber]->getDefaultKeys()));
        dialogOption.launchAsync();
    }
}

//===------------------------------------------------------------------===//
// ComponentWithTable
//===------------------------------------------------------------------===//
String ColorEditor::getText(const int rowNumber, const int columnNumber) const
{
    String text;
    auto* entry = (*project.getPitchColorMap())[rowNumber];
    switch (columnNumber)
    {
    case 1: text = entry->getName(); break;
    case 3: text = String(entry->getValue()); break;
    default:
        break;
    }
    return text;
}

void ColorEditor::setText(const int rowNumber, const int columnNumber, const String& newText)
{
    auto* entry = (*project.getPitchColorMap())[rowNumber];
    if (entry->getName() == "0")
        return;
    switch (columnNumber)
    {
    case 1:
    {
        if (entry->getName() != newText)
        {
            processor.getUndoManager().beginNewTransaction("'Change pitch color entry's name'");
            project.getPitchColorMap()->change(*entry, entry->withName(newText), true);
        }
        break;
    }
    case 3:
    {
        if (entry->getValue() != newText.getIntValue())
        {
            processor.getUndoManager().beginNewTransaction("'Change pitch color entry's value'");
            project.getPitchColorMap()->change(*entry, entry->withValue(newText.getIntValue()), true);
        }
        break;
    }
    default:
        return;
    }
}

void ColorEditor::colorChooserClosed(int rowNumber, int columnId, const Colour& color)
{
    processor.getUndoManager().beginNewTransaction("'Change pitch color entry's color'");
    project.getPitchColorMap()->change(*(*project.getPitchColorMap())[rowNumber], 
        (*project.getPitchColorMap())[rowNumber]->withColor(color), true);
}

void ColorEditor::defaultKeyChooserClosed(int rowNumber, int columnId, const std::unordered_set<int>& defaultKeys)
{
    processor.getUndoManager().beginNewTransaction("'Change pitch color entry's default keys'");
    project.getPitchColorMap()->change(*(*project.getPitchColorMap())[rowNumber], 
        (*project.getPitchColorMap())[rowNumber]->withDefaultKeys(defaultKeys), true);
}

//===------------------------------------------------------------------===//
// Project Listener
//===------------------------------------------------------------------===//
void ColorEditor::onPostAddPitchColorMapEntry()
{
    table.updateContent();
    table.repaint();
    checkModified();
}

void ColorEditor::onPostChangePitchColorMapEntry()
{
    table.updateContent();
    table.repaint();
    checkModified();
}

void ColorEditor::onPostRemovePitchColorMapEntry()
{
    table.updateContent();
    table.repaint();
    checkModified();
}
void ColorEditor::onChangePitchColorMap(PitchColorMap* const colorMap)
{
    table.updateContent();
    table.repaint();
    checkModified();
}

void ColorEditor::onReloadProjectContent(const Array<MidiTrack*>& tracks)
{
    table.updateContent();
    table.repaint();
    checkModified();
}

//===------------------------------------------------------------------===//
// Helpers
//===------------------------------------------------------------------===//
void ColorEditor::updatePresetComboBox(String name)
{
    processor.setSelectedColorPresetName(name);
    presetComboBox.clear(dontSendNotification);
    for (auto* m : colorMapPresets)
        presetComboBox.addItem(m->getName(), m->getId());
    if (auto* m = findColorMapByName(name))
        presetComboBox.setSelectedId(m->getId(), true);
    else if(auto* m = findColorMapByName("---INIT---"))
        presetComboBox.setSelectedId(m->getId(), true);
    checkModified();
}

PitchColorMap* ColorEditor::findColorMapByName(String name)
{
    for (auto* m : colorMapPresets)
        if (m->getName() == name)
            return m;
    return nullptr;
}

void ColorEditor::checkModified()
{
    if (presetComboBox.getSelectedItemIndex() > -1)
    {
        hasPresetModified = false;
        auto* selected = colorMapPresets[presetComboBox.getSelectedItemIndex()];
        auto* current = project.getPitchColorMap();
        if (selected->size() != current->size())
        {
            hasPresetModified = true;
        }
        else
        {
            for (int i = 0; i < current->size(); i++)
            {
                if (!PitchColorMapEntry::equalWithoutId((*current)[i], (*selected)[i]))
                {
                    hasPresetModified = true;
                    break;
                }
            }
        }
        savePresetBtn.setEnabled(hasPresetModified);
        if (selected->getName() == "---INIT---")
        {
            renamePresetBtn.setEnabled(false);
            deletePresetBtn.setEnabled(false);
        }
        else
        {
            renamePresetBtn.setEnabled(true);
            deletePresetBtn.setEnabled(true);
        }
    }
    else
    {
        hasPresetModified = false;
        savePresetBtn.setEnabled(true);
    }
}

//===------------------------------------------------------------------===//
// Presets
//===------------------------------------------------------------------===//
void ColorEditor::loadColorPreset()
{
    bool hasInit = false;
    colorMapPresets.clear();
    if (appProperties.getUserSettings()->containsKey(Serialization::PitchColor::colorMapPresets.toString()))
    {
        auto rootXml = appProperties.getUserSettings()->getXmlValue(Serialization::PitchColor::colorMapPresets.toString());
        auto tree = ValueTree::fromXml(*rootXml);

        const auto root =
            tree.hasType(Serialization::PitchColor::colorMapPresets) ?
            tree : tree.getChildWithName(Serialization::PitchColor::colorMapPresets);

        if (root.isValid())
        {
            for (const auto& e : root)
            {
                if (e.hasType(Serialization::PitchColor::colorMap))
                {
                    auto* m = new PitchColorMap(nullptr);
                    m->deserialize(e);
                    m->sort();
                    if (m->getName() == "---INIT---")
                        hasInit = true;
                    colorMapPresets.add(m);
                }
            }
        }
    }
    if (!hasInit)
        colorMapPresets.add(new PitchColorMap());
    colorMapPresets.sort(PitchColorMapNameComparator(), true);
}

void ColorEditor::saveColorMapPresets()
{
    ValueTree root(Serialization::PitchColor::colorMapPresets);
    for (auto* m : colorMapPresets)
        root.appendChild(m->serialize(), nullptr);
    appProperties.getUserSettings()->setValue(Serialization::PitchColor::colorMapPresets.toString(),
        var(root.createXml()->toString(XmlElement::TextFormat().singleLine().withoutHeader())));
    appProperties.saveIfNeeded();
}

//===------------------------------------------------------------------===//
// Components
//===------------------------------------------------------------------===//
void ColorEditor::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void ColorEditor::resized()
{
    auto b = getLocalBounds();
    b.reduce(10, 10);
    auto half = b.proportionOfWidth(1.0 / 2);
    auto fifth = b.proportionOfWidth(1.0 / 5);

    auto tmp = b.removeFromTop(28);
    b.removeFromTop(4);
    deletePresetBtn.setBounds(tmp.removeFromRight(fifth).reduced(2));
    renamePresetBtn.setBounds(tmp.removeFromRight(fifth).reduced(2));
    savePresetBtn.setBounds(tmp.removeFromRight(fifth).reduced(2));
    presetComboBox.setBounds(tmp.reduced(2));

    tmp = b.removeFromBottom(28);
    b.removeFromBottom(4);
    addEntryBtn.setBounds(tmp.removeFromLeft(half).reduced(2));
    removeEntryBtn.setBounds(tmp.removeFromLeft(half).reduced(2));

    table.setBounds(b);
}
