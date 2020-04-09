/*
  ==============================================================================

    Common.h
    Created: 25 Mar 2020 9:36:47pm
    Author:  bowen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class PluginBundle;
class MicroChromoAudioProcessor;
class MicroChromoAudioProcessorEditor;

enum
{
    mainMenuIdBase = 0x2717,
    pluginMenuIdBase = 0x324503f4
};

enum
{
    SLOT_MENU_SHOW_MAIN_GUI = 0 + mainMenuIdBase,
    SLOT_MENU_SHOW_TWO_GUI = 1 + mainMenuIdBase,
    SLOT_MENU_SHOW_ALL_GUI = 2 + mainMenuIdBase,
    SLOT_MENU_CLOSE_ALL_GUI = 3 + mainMenuIdBase,
    SLOT_MENU_SHOW_PROGRAMS = 4 + mainMenuIdBase,
    SLOT_MENU_SHOW_PARAMETERS = 5 + mainMenuIdBase,
    SLOT_MENU_SHOW_DEBUG_LOG = 6 + mainMenuIdBase,
    SLOT_MENU_PROPAGATE_STATE = 7 + mainMenuIdBase,
    SLOT_MENU_EXPOSE_PARAMETER = 8 + mainMenuIdBase,
    SLOT_MENU_START_CC = 9 + mainMenuIdBase,
    SLOT_MENU_SHOW_CC = 10 + mainMenuIdBase,
    SLOT_MENU_CLEAR_CC = 11 + mainMenuIdBase,
    SLOT_MENU_LOAD_EMPTY_PLUGIN = 12 + mainMenuIdBase,
    SLOT_MENU_LOAD_DEFAULT_PLUGIN = 13 + mainMenuIdBase,
    SLOT_MENU_USE_KONTAKT = 14 + mainMenuIdBase,
    SLOT_MENU_COPY_KONTAKT_SCRIPT = 15 + mainMenuIdBase,

    PLUGIN_SORT_MANUFACTURER = 16 + mainMenuIdBase,
    PLUGIN_SORT_CATEGORY = 17 + mainMenuIdBase,
    PLUGIN_SORT_ALPHABETICALLY = 18 + mainMenuIdBase,
    PLUGIN_SORT_FORMAT = 19 + mainMenuIdBase,

    CC_VALUE_BASE = 20 + mainMenuIdBase
};

enum CommandIDs
{
    openPluginScanner = 1,
    testCommand
};

enum
{
    USE_NONE = 1,
    USE_PS,
    USE_SYNTH,
    USE_KONTAKT
};

class AudioParameterFloatVariant : public AudioParameterFloat
{
public:
    AudioParameterFloatVariant(const String& parameterID,
        const String& parameterName,
        NormalisableRange<float> normalisableRange,
        float defaultValue,
        const String& parameterLabel = String(),
        Category parameterCategory = AudioProcessorParameter::genericParameter,
        std::function<String(float value, int maximumStringLength)> stringFromValue = nullptr,
        std::function<float(const String & text)> valueFromString = nullptr);
    AudioParameterFloatVariant(String parameterID, String parameterName, float minValue, float maxValue, float defaultValue);

    void setName(const String newName);

protected:
    void valueChanged(float newValue) override;
};

class ParameterLinker
{
    using ABool = std::atomic<bool>;
public:
    ParameterLinker(AudioParameterFloatVariant* genericParameter, std::shared_ptr<PluginBundle>& bundle);
    ~ParameterLinker();

    void linkParameter(int index, AudioProcessorParameter* parameter);
    void stateReset();
    void resetLink();

private:
    class OneToOneListener : public AudioProcessorParameter::Listener
    {
    public:
        OneToOneListener(ABool& thisLockValue, ABool& thisLockGesture, ABool& thisGestureOverride,
            ABool& otherLockValue, ABool& otherLockGesture, ABool& otherGestureOverride, 
            AudioProcessorParameter* paramSource, AudioProcessorParameter* paramDest);
        ~OneToOneListener();

        void parameterValueChanged(int /*parameterIndex*/, float newValue) override;
        void parameterGestureChanged(int /*parameterIndex*/, bool gestureIsStarting) override;

    private:
        ABool& _thisLockValue, & _thisLockGesture, & _thisGestureOverride, & _otherLockValue, & _otherLockGesture, & _otherGestureOverride;
        AudioProcessorParameter* _paramSource, * _paramDest;
    };

    class OneToManyListener : public AudioProcessorParameter::Listener
    {
    public:
        OneToManyListener(ABool& thisLockValue, ABool& thisLockGesture, ABool& thisGestureOverride,
            ABool& otherLockValue, ABool& otherLockGesture, ABool& otherGestureOverride,
            AudioProcessorParameter* paramSource, std::shared_ptr<PluginBundle>& bundle, int index);
        ~OneToManyListener();

        void parameterValueChanged(int /*parameterIndex*/, float newValue) override;
        void parameterGestureChanged(int /*parameterIndex*/, bool gestureIsStarting) override;

    private:
        ABool& _thisLockValue, & _thisLockGesture, & _thisGestureOverride, & _otherLockValue, & _otherLockGesture, & _otherGestureOverride;
        AudioProcessorParameter* _paramSource;
        std::shared_ptr<PluginBundle>& _bundle;
        int _index;
    };

    std::shared_ptr<PluginBundle>& _bundle;
    std::atomic<bool> aLockValue{ true }, aLockGesture{ true }, bLockValue{ true }, bLockGesture{ true }, aGestureOverride{ true }, bGestureOverride{ true };
    AudioParameterFloatVariant* _genericParameter;
    AudioProcessorParameter* _parameter;
    int _index;
    std::unique_ptr<AudioProcessorParameter::Listener> listener1{ nullptr }, listener2{ nullptr };
};

class ComponentWithTable : public Component, public TableListBoxModel
{
public:
    ComponentWithTable(MicroChromoAudioProcessorEditor& editor);
    ~ComponentWithTable() = default;

    void selectedRowsChanged(int row) override;

    virtual String getText(const int rowNumber, const int columnNumber) const { return ""; };
    virtual void setText(const int rowNumber, const int columnNumber, const String& newText) {};

    virtual int getSelectedId(const int rowNumber, const int columnNumber) const { return -1; };
    virtual void setSelectedId(const int rowNumber, const int columnNumber, const int newId) {};

    virtual void buttonClicked(const int rowNumber, const int columnNumber, const Colour& newColor) {};

    virtual void updateColorMapList() {}

protected:
    MicroChromoAudioProcessorEditor& owner;
    MicroChromoAudioProcessor& processor;
    TableListBox table{ {}, this };
    Font font{ 14.0f };

    int lastRow = -1;

    HashMap<String, int> itemTextToitemId;
    HashMap<int, String> itemIdToitemText;
    Array<String> itemOrder;

    friend class EditableTextCustomComponent;
    friend class ComboBoxCustomComponent;
    friend class TextButtonCustomComponent;
};

class EditableTextCustomComponent : public Label
{
public:
    EditableTextCustomComponent(ComponentWithTable& td);
    void mouseDown(const MouseEvent& event) override;
    void textWasEdited() override;
    void setRowAndColumn(const int newRow, const int newColumn);
private:
    ComponentWithTable& owner;
    int rowId{ -1 }, columnId{ -1 };
};

class ComboBoxCustomComponent : public ComboBox, ComboBox::Listener
{
public:
    ComboBoxCustomComponent(ComponentWithTable& td);
    ~ComboBoxCustomComponent();

    void updateList();
    void setRowAndColumn(const int newRow, const int newColumn);
    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;
private:
    ComponentWithTable& owner;
    int rowId{ -1 }, columnId{ -1 };
};

class TextButtonCustomComponent : public TextButton, Button::Listener
{
public:
    TextButtonCustomComponent(ComponentWithTable& td);
    ~TextButtonCustomComponent();

    void updateColor(Colour newColor);
    void buttonClicked(Button* btn);
    void setRowAndColumn(const int newRow, const int newColumn);
private:
    ComponentWithTable& owner;
    int rowId{ -1 }, columnId{ -1 };
    Colour color;
};

struct SimpleMidiMessage
{
    SimpleMidiMessage(int channel, int key, float timestamp, float velocity, int pitchbend, int cc, bool isNoteOn, float adjustment);

    String toString() const;

    MidiMessage noteMsg, ccMsg;
    bool noteOn = false;
};

struct SimpleMidiMessageComparator
{
    SimpleMidiMessageComparator() = default;

    static int compareElements(const SimpleMidiMessage& first, const SimpleMidiMessage& second)
    {
        const float timeDiff = first.noteMsg.getTimeStamp() - second.noteMsg.getTimeStamp();
        return (timeDiff > 0.f) - (timeDiff < 0.f);
    }
};

class ColorPitchBendRecord
{
public:
    ColorPitchBendRecord();
    ColorPitchBendRecord(String name, int value, Colour color);

    String name;
    int value;
    Colour color;
};

struct ColorPitchBendRecordComparator
{
    ColorPitchBendRecordComparator() = default;

    static int compareElements(const ColorPitchBendRecord& first, const ColorPitchBendRecord& second)
    {
        const float valDiff = first.value - second.value;
        return (valDiff > 0.f) - (valDiff < 0.f);
    }
};
