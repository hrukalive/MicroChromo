/*
  ==============================================================================

    ColorEditor.cpp
    Created: 8 Apr 2020 9:50:04am
    Author:  bowen

  ==============================================================================
*/

#include "ColorEditor.h"
#include "SimpleMidiEditor.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Note.h"

//==============================================================================
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

//==============================================================================
ColorEditor::ColorEditor(MicroChromoAudioProcessorEditor& editor, SimpleMidiEditor& midiEditor) :
    owner(editor), processor(editor.getProcessor()),
    appProperties(processor.getApplicationProperties()), 
    _midiEditor(midiEditor), 
    noteColorMap(processor.getNoteColorMap())
{
    loadColorPreset();

    int columnId = 1;
    table.getHeader().addColumn("Name", columnId++, 120, 100, 200, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("Color", columnId++, 50, 50, 100, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("Value", columnId++, 70, 60, 120, TableHeaderComponent::notSortable | TableHeaderComponent::sortedForwards);

    table.setColour(ListBox::outlineColourId, Colours::grey);
    table.setOutlineThickness(1);
    table.setMultipleSelectionEnabled(false);
    table.getHeader().setSortColumnId(3, true);

    addAndMakeVisible(table);

    addAndMakeVisible(addBtn);
    addBtn.onClick = [&]() {
        int i = 1;
        for (; noteColorMap.contains("New" + String(i)); i++);
        noteColorMap.set("New" + String(i), ColorPitchBendRecord("New" + String(i), 0, Colours::grey));
        updateColorMapList();
        _midiEditor.updateColorMapList();
        hasPresetModified = true;
        if (presetComboBox.getSelectedId() > 0)
            presetComboBox.setText("*" + presetComboBox.getText());
    };

    addAndMakeVisible(removeBtn);
    removeBtn.onClick = [&]() {
        if (lastRow > -1 && noteColorMapList[lastRow].name != "0")
        {
            noteColorMap.remove(noteColorMapList[lastRow].name);
            updateColorMapList();
            processor.filterNotesWithColorMap();
            _midiEditor.updateColorMapList();
            hasPresetModified = true;
            if (presetComboBox.getSelectedId() > 0)
                presetComboBox.setText("*" + presetComboBox.getText());
        }
    };

    addAndMakeVisible(presetComboBox);
    presetComboBox.onChange = [&]() {
        if (presetComboBox.getSelectedId() > 0)
            useColorPreset(colorMapPresetsList.getReference(presetComboBox.getSelectedId() - 1).name);
    };

    addAndMakeVisible(saveBtn);
    saveBtn.onClick = [&]() {
        presetNamePrompt = new AlertWindow("Add Preset", "Name", AlertWindow::AlertIconType::NoIcon);
        presetNamePrompt->addTextEditor("nameEditor", "", "", false);
        presetNamePrompt->addButton("OK", 1, KeyPress(KeyPress::returnKey, 0, 0));
        presetNamePrompt->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey, 0, 0));
        presetNamePrompt->enterModalState(true, ModalCallbackFunction::create([this](int r) {
            if (r)
            {
                auto name = presetNamePrompt->getTextEditorContents("nameEditor");
                addColorPreset(name, noteColorMapList);
            }
        }), true);
    };

    addAndMakeVisible(deleteBtn);
    deleteBtn.onClick = [&]() {
        removeColorPreset(colorMapPresetsList.getReference(presetComboBox.getSelectedId() - 1).name);
    };

    updateColorMapList();
    selectPresetWithName(processor.getSelectedColorPresetName());
    checkModified();

    setSize(400, 300);
}

ColorEditor::~ColorEditor()
{
}

int ColorEditor::getNumRows()
{
    return noteColorMapList.size();
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
    if (columnId == 2)
    {
        g.setColour(noteColorMapList[rowNumber].color);
        g.fillRect(4, 4, width - 8, height - 8);
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
        return textLabel;
    }

    jassert(existingComponentToUpdate == nullptr);
    return nullptr;
}

void ColorEditor::cellDoubleClicked(int rowNumber, int columnId, const MouseEvent&)
{
    if (columnId == 2)
    {
        DialogWindow::LaunchOptions dialogOption;

        dialogOption.dialogTitle = "Color Selector";
        dialogOption.dialogBackgroundColour = LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId);
        dialogOption.escapeKeyTriggersCloseButton = false;
        dialogOption.useNativeTitleBar = false;
        dialogOption.resizable = true;
        dialogOption.content.setOwned(new ColorChooserComponent(*this, rowNumber, columnId, noteColorMapList[rowNumber].color));
        dialogOption.launchAsync();
    }
}

int ColorEditor::getColumnAutoSizeWidth(int columnId)
{
    int widest = 92;

    for (auto i = getNumRows(); --i >= 0;)
    {

        String text;
        switch (columnId)
        {
        case 1: text = noteColorMapList[i].name; break;
        case 3: text = String(noteColorMapList[i].value); break;
        default:
            break;
        }
        widest = jmax(widest, font.getStringWidth(text));
    }

    return widest + 8;
}

String ColorEditor::getText(const int rowNumber, const int columnNumber) const
{
    String text;
    switch (columnNumber)
    {
    case 1: text = noteColorMapList[rowNumber].name; break;
    case 3: text = String(noteColorMapList[rowNumber].value); break;
    default:
        break;
    }
    return text;
}

void ColorEditor::setText(const int rowNumber, const int columnNumber, const String& newText)
{
    if (noteColorMapList[rowNumber].name == "0" || (columnNumber == 1 && noteColorMap.contains(newText)))
    {
        table.updateContent();
        return;
    }
    switch (columnNumber)
    {
    case 1: processor.renameNoteColorMap(noteColorMapList[rowNumber].name, newText); updateColorMapList(); _midiEditor.updateColorMapList(); break;
    case 3: noteColorMap.getReference(noteColorMapList[rowNumber].name).value = newText.getIntValue(); updateColorMapList(); _midiEditor.updateColorMapList(); break;
    default:
        break;
    }
    hasPresetModified = true;
    if (presetComboBox.getSelectedId() > 0)
        presetComboBox.setText("*" + presetComboBox.getText());
}

void ColorEditor::colorChooserClosed(int rowNumber, int columnId, const Colour& color)
{
    noteColorMap.getReference(noteColorMapList[rowNumber].name).color = color;
    updateColorMapList();
    _midiEditor.updateColorMapList();
    hasPresetModified = true;
    if (presetComboBox.getSelectedId() > 0)
        presetComboBox.setText("*" + presetComboBox.getText());
}

void ColorEditor::updateColorMapList()
{
    noteColorMapList.clear();

    HashMap<String, ColorPitchBendRecord>::Iterator i(processor.getNoteColorMap());
    while (i.next())
        noteColorMapList.add(i.getValue());

    noteColorMapList.sort(ColorPitchBendRecordComparator());

    table.updateContent();
    table.repaint();
}

void ColorEditor::updateColorMapPresetList()
{
    colorMapPresetsList.clear();

    HashMap<String, ColorPitchBendRecordCollection>::Iterator it(colorMapPresets);
    while (it.next())
        colorMapPresetsList.add(it.getValue());

    colorMapPresetsList.sort(ColorPitchBendRecordCollectionComparator());

    updatePresetComboBox();
}

void ColorEditor::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void ColorEditor::resized()
{
    auto b = getLocalBounds();
    b.reduce(10, 10);
    auto half = b.proportionOfWidth(0.48);
    auto space = b.proportionOfWidth(0.04);

    auto tmp = b.removeFromBottom(24);
    b.removeFromBottom(space);
    addBtn.setBounds(tmp.removeFromLeft(half));
    tmp.removeFromLeft(space);
    removeBtn.setBounds(tmp.removeFromLeft(half));

    tmp = b.removeFromTop(24);
    b.removeFromTop(space);
    auto tmp2 = tmp.proportionOfWidth(0.7);
    auto tmp3 = tmp.proportionOfWidth(0.14);
    auto tmp4 = tmp.proportionOfWidth(0.01);
    presetComboBox.setBounds(tmp.removeFromLeft(tmp2));
    tmp.removeFromLeft(tmp4);
    saveBtn.setBounds(tmp.removeFromLeft(tmp3));
    tmp.removeFromLeft(tmp4);
    deleteBtn.setBounds(tmp.removeFromLeft(tmp3));

    table.setBounds(b);
}

void ColorEditor::loadColorPreset()
{
    colorMapPresets.clear();
    if (appProperties.getUserSettings()->containsKey("colorMapPresets"))
    {
        auto root = appProperties.getUserSettings()->getXmlValue("colorMapPresets");
        forEachXmlChildElementWithTagName(*root, child, "colorMap")
        {
            auto name = child->getStringAttribute("name", "Unknown");
            if (name.isNotEmpty())
            {
                Array<ColorPitchBendRecord> colors;
                forEachXmlChildElementWithTagName(*child, record, "record")
                {
                    colors.add(ColorPitchBendRecord(record->getStringAttribute("name", "Unknown"),
                        record->getDoubleAttribute("value", 0),
                        Colour::fromString(record->getStringAttribute("color", "0x000000"))));
                }
                colorMapPresets.set(name, ColorPitchBendRecordCollection(name, colors));
            }
        }
    }
    colorMapPresets.set("---INIT---", ColorPitchBendRecordCollection());

    updateColorMapPresetList();
}

void ColorEditor::useColorPreset(String name)
{
    if (colorMapPresets.contains(name))
    {
        processor.setSelectedColorPresetName(name);
        processor.updateNoteColorMap(colorMapPresets[name].collection);
        updateColorMapList();
        _midiEditor.updateColorMapList();
    }
    hasPresetModified = false;
}

void ColorEditor::addColorPreset(String name, Array<ColorPitchBendRecord> colors)
{
    processor.setSelectedColorPresetName(name);
    colorMapPresets.set(name, { name, colors });
    saveColorMapPresets();
    hasPresetModified = false;
    updateColorMapPresetList();
}

void ColorEditor::removeColorPreset(String name)
{
    if (name == "---INIT---")
        return;
    colorMapPresets.remove(name);
    saveColorMapPresets();
    updateColorMapPresetList();
}

void ColorEditor::updatePresetComboBox()
{
    presetComboBox.clear();
    int i = 1;
    for (auto& c : colorMapPresetsList)
    {
        presetComboBox.addItem(c.name, i);
        i++;
    }
    selectPresetWithName(processor.getSelectedColorPresetName());
}

void ColorEditor::selectPresetWithName(String name)
{
    if (!colorMapPresets.contains(name))
        name = "---INIT---";

    for (int i = 0; i < colorMapPresetsList.size(); i++)
    {
        if (colorMapPresetsList[i].name == name)
        {
            presetComboBox.setSelectedId(i + 1, dontSendNotification);
            break;
        }
    }
    checkModified();
}

void ColorEditor::checkModified()
{
    if (presetComboBox.getSelectedId() > 0)
    {
        auto& selected = colorMapPresetsList.getReference(presetComboBox.getSelectedId() - 1);
        if (selected.collection.size() != noteColorMapList.size())
        {
            hasPresetModified = true;
            presetComboBox.setText("*" + presetComboBox.getText());
        }
        else
        {
            auto colors = selected.collection;
            colors.sort(ColorPitchBendRecordComparator());
            for (int i = 0; i < noteColorMapList.size(); i++)
            {
                if (ColorPitchBendRecordComparator::compareElements(colors.getReference(i), noteColorMapList.getReference(i)) != 0)
                {
                    hasPresetModified = true;
                    presetComboBox.setText("*" + presetComboBox.getText());
                    break;
                }
            }
        }
    }
}

void ColorEditor::saveColorMapPresets()
{
    std::unique_ptr<XmlElement> root{ new XmlElement("colorMapPresets") };
    HashMap<String, ColorPitchBendRecordCollection>::Iterator it(colorMapPresets);
    while (it.next())
        root->addChildElement(ColorPitchBendRecordCollection::getColorMapXml(it.getKey(), it.getValue().collection));
    appProperties.getUserSettings()->setValue("colorMapPresets", root.get());
    appProperties.saveIfNeeded();
}
