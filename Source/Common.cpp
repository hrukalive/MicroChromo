/*
  ==============================================================================

    Common.cpp
    Created: 25 Mar 2020 9:36:47pm
    Author:  bowen

  ==============================================================================
*/

#include "Common.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PluginBundle.h"

AudioParameterFloatVariant::AudioParameterFloatVariant(const String& parameterID,
    const String& parameterName,
    NormalisableRange<float> normalisableRange,
    float defaultValue,
    const String& parameterLabel,
    Category parameterCategory,
    std::function<String(float value, int maximumStringLength)> stringFromValue,
    std::function<float(const String & text)> valueFromString) :
    AudioParameterFloat(parameterID,
        parameterName,
        normalisableRange,
        defaultValue,
        parameterLabel,
        parameterCategory,
        stringFromValue,
        valueFromString)
{}

AudioParameterFloatVariant::AudioParameterFloatVariant(String parameterID, String parameterName, float minValue, float maxValue, float defaultValue) :
    AudioParameterFloat(parameterID, parameterName, minValue, maxValue, defaultValue)
{}

void AudioParameterFloatVariant::setName(const String newName)
{
    name = newName;
}

void AudioParameterFloatVariant::valueChanged(float newValue)
{
    sendValueChangedMessageToListeners(convertTo0to1(newValue));
}


ComponentWithTable::ComponentWithTable() {}

EditableTextCustomComponent::EditableTextCustomComponent(ComponentWithTable& td)
    : owner(td)
{
    setEditable(false, true, false);
}
void EditableTextCustomComponent::mouseDown(const MouseEvent& event)
{
    owner.table.selectRowsBasedOnModifierKeys(rowId, event.mods, false);
    Label::mouseDown(event);
}
void EditableTextCustomComponent::textWasEdited()
{
    owner.setText(rowId, columnId, getText());
}
void EditableTextCustomComponent::setRowAndColumn(const int newRow, const int newColumn)
{
    rowId = newRow;
    columnId = newColumn;
    setText(owner.getText(rowId, columnId), dontSendNotification);
}

ComboBoxCustomComponent::ComboBoxCustomComponent(ComponentWithTable& td)
    : owner(td)
{
    addListener(this);
}
ComboBoxCustomComponent::~ComboBoxCustomComponent()
{
    removeListener(this);
}
void ComboBoxCustomComponent::mouseDown(const MouseEvent& event)
{
    owner.table.selectRowsBasedOnModifierKeys(rowId, event.mods, false);
    ComboBox::mouseDown(event);
}
void ComboBoxCustomComponent::setRowAndColumn(const int newRow, const int newColumn)
{
    rowId = newRow;
    columnId = newColumn;
    setSelectedId(owner.getSelectedId(rowId, columnId), dontSendNotification);
}
void ComboBoxCustomComponent::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    owner.setSelectedId(rowId, columnId, getSelectedId());
}
