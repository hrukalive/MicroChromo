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

ParameterLinker::ParameterLinker(AudioParameterFloatVariant* genericParameter, std::shared_ptr<PluginBundle>& bundle)
    : _genericParameter(genericParameter), _bundle(bundle)
{}
ParameterLinker::~ParameterLinker()
{
    listener1 = nullptr;
    listener2 = nullptr;
    _genericParameter = nullptr;
    _parameter = nullptr;
    _bundle = nullptr;
}

void ParameterLinker::linkParameter(int index, AudioProcessorParameter* parameter)
{
    _genericParameter->setName(parameter->getName(128));
    dynamic_cast<AudioProcessorParameter*>(_genericParameter)->setValue(parameter->getValue());
    _parameter = parameter;
    _index = index;
    listener1 = std::make_unique<OneToManyListener>(aLockValue, aLockGesture, aGestureOverride, bLockValue, bLockGesture, bGestureOverride, _genericParameter, _bundle, _index);
    listener2 = std::make_unique<OneToOneListener>(bLockValue, bLockGesture, bGestureOverride, aLockValue, aLockGesture, aGestureOverride, _parameter, _genericParameter);
}

void ParameterLinker::stateReset()
{
    aLockValue = true;
    aLockGesture = true;
    aGestureOverride = true;
    bLockValue = true;
    bLockGesture = true;
    bGestureOverride = true;
}

void ParameterLinker::resetLink()
{
    listener1 = nullptr;
    listener2 = nullptr;
    _genericParameter->setName("");
    _parameter = nullptr;
    aLockValue = true;
    aLockGesture = true;
    bLockValue = true;
    bLockGesture = true;
}

ParameterLinker::OneToOneListener::OneToOneListener(
    ABool& thisLockValue, ABool& thisLockGesture, ABool& thisGestureOverride,
    ABool& otherLockValue, ABool& otherLockGesture, ABool& otherGestureOverride,
    AudioProcessorParameter* paramSource, AudioProcessorParameter* paramDest)
    : _thisLockValue(thisLockValue), _thisLockGesture(thisLockGesture), _thisGestureOverride(thisGestureOverride), 
    _otherLockValue(otherLockValue), _otherLockGesture(otherLockGesture), _otherGestureOverride(otherGestureOverride), 
    _paramSource(paramSource), _paramDest(paramDest)
{
    _paramSource->addListener(this);
}
ParameterLinker::OneToOneListener::~OneToOneListener()
{
    _paramSource->removeListener(this);
}

void ParameterLinker::OneToOneListener::parameterValueChanged(int /*parameterIndex*/, float newValue)
{
    if (_thisGestureOverride)
    {
        if (_thisLockValue)
        {
            _otherLockValue = false;
            _paramDest->setValue(newValue);
        }
        else
            _thisLockValue = true;
    }
}
void ParameterLinker::OneToOneListener::parameterGestureChanged(int /*parameterIndex*/, bool gestureIsStarting)
{
    if (_thisLockGesture)
    {
        _otherLockGesture = false;
        if (gestureIsStarting)
        {
            _otherGestureOverride = false;
            _paramDest->beginChangeGesture();
        }
        else
        {
            _otherGestureOverride = true;
            _paramDest->endChangeGesture();
        }
    }
    else
        _thisLockGesture = true;
}

ParameterLinker::OneToManyListener::OneToManyListener(
    ABool& thisLockValue, ABool& thisLockGesture, ABool& thisGestureOverride,
    ABool& otherLockValue, ABool& otherLockGesture, ABool& otherGestureOverride,
    AudioProcessorParameter* paramSource, std::shared_ptr<PluginBundle>& bundle, int index)
    : _thisLockValue(thisLockValue), _thisLockGesture(thisLockGesture), _thisGestureOverride(thisGestureOverride),
    _otherLockValue(otherLockValue), _otherLockGesture(otherLockGesture), _otherGestureOverride(otherGestureOverride),
    _paramSource(paramSource), _bundle(bundle), _index(index)
{
    _paramSource->addListener(this);
}
ParameterLinker::OneToManyListener::~OneToManyListener()
{
    _paramSource->removeListener(this);
}

void ParameterLinker::OneToManyListener::parameterValueChanged(int /*parameterIndex*/, float newValue)
{
    if (_thisGestureOverride)
    {
        if (_thisLockValue)
        {
            _otherLockValue = false;
            _bundle->setParameterValue(_index, newValue);
        }
        else
            _thisLockValue = true;
    }
}
void ParameterLinker::OneToManyListener::parameterGestureChanged(int /*parameterIndex*/, bool gestureIsStarting)
{
    if (_thisLockGesture)
    {
        _otherLockGesture = false;
        _otherGestureOverride = !gestureIsStarting;
        _bundle->setParameterGesture(_index, gestureIsStarting);
    }
    else
        _thisLockGesture = true;
}

ComponentWithTable::ComponentWithTable(MicroChromoAudioProcessorEditor& editor) : 
    owner(editor), processor(editor.getProcessor()) {}

void ComponentWithTable::selectedRowsChanged(int row)
{
    lastRow = row;
}

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
void ComboBoxCustomComponent::updateList()
{
    clear();
    for (auto& k : owner.itemOrder)
        addItem(k, owner.itemTextToitemId[k]);
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

TextButtonCustomComponent::TextButtonCustomComponent(ComponentWithTable& td)
    : owner(td)
{
    addListener(this);
}
TextButtonCustomComponent::~TextButtonCustomComponent()
{
    removeListener(this);
}
void TextButtonCustomComponent::updateColor(Colour newColor)
{
    color = newColor;
}
void TextButtonCustomComponent::setRowAndColumn(const int newRow, const int newColumn)
{
    rowId = newRow;
    columnId = newColumn;
}
void TextButtonCustomComponent::buttonClicked(Button* btn)
{
    owner.buttonClicked(rowId, columnId, color);
}

SimpleMidiMessage::SimpleMidiMessage(int channel, int key, float timestamp, float velocity, int pitchbend, int cc, bool isNoteOn, float adjustment)
{
    if (isNoteOn)
    {
        noteMsg = MidiMessage::noteOn(channel, key, velocity).withTimeStamp(timestamp);
        ccMsg = MidiMessage::controllerEvent(channel, cc, pitchbend + 50).withTimeStamp(jmax(0.0f, timestamp - adjustment));
    }
    else
    {
        noteMsg = MidiMessage::noteOff(channel, key, velocity).withTimeStamp(timestamp);
    }
    noteOn = isNoteOn;
}

String SimpleMidiMessage::toString() const
{
    return (noteOn ? " ON " : " OFF ") + ("key: " + String(noteMsg.getNoteNumber())) + 
        " time: " + String(noteMsg.getTimeStamp(), 2) + 
        " vel: " + String(noteMsg.getVelocity()) +
        (noteOn ? (" pit: " + String(ccMsg.getControllerValue() - 50) + " cc: " + String(ccMsg.getControllerNumber())) : "");
}

ColorPitchBendRecord::ColorPitchBendRecord() :
    name("Original"), value(0), color(Colours::grey) {}
ColorPitchBendRecord::ColorPitchBendRecord(String name, int value, Colour color) :
    name(name), value(value), color(color) {}

ColorPitchBendRecordCollection::ColorPitchBendRecordCollection() :
    name("---INIT---"), collection({ ColorPitchBendRecord("0", 0, Colours::grey) }) {}
ColorPitchBendRecordCollection::ColorPitchBendRecordCollection(String name, const Array<ColorPitchBendRecord>& colors) :
    name(name), collection(colors) {}

XmlElement* ColorPitchBendRecordCollection::getColorMapXml(const String& name, const Array<ColorPitchBendRecord>& colors)
{
    XmlElement* root{ new XmlElement("colorMap") };
    root->setAttribute("name", name);
    for (auto& c : colors)
    {
        auto* line = root->createNewChildElement("record");
        line->setAttribute("name", c.name);
        line->setAttribute("value", c.value);
        line->setAttribute("color", c.color.toString());
    }
    return root;
}
