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

#pragma once

//===----------------------------------------------------------------------===//
// Pragmas
//===----------------------------------------------------------------------===//
// unreferenced formal parameter
#pragma warning(disable: 4100)
// hides class member
#pragma warning(disable: 4458)
// decorated name length exceeded, name was truncated
#pragma warning(disable: 4503)
// conditional expression is constant
#pragma warning(disable: 4127)

#if JUCE_LINUX
#   define JUCE_USE_FREETYPE_AMALGAMATED 1
#endif

#include <JuceHeader.h>

#define BEATS_PER_BAR 4
#define TICKS_PER_BEAT 16
#define VELOCITY_SAVE_ACCURACY 1024

//===----------------------------------------------------------------------===//
// Utilities
//===----------------------------------------------------------------------===//
inline float roundBeat(float beat)
{
    return jmax(0.0f, roundf(beat * static_cast<float>(TICKS_PER_BEAT)) / static_cast<float>(TICKS_PER_BEAT));
}

//===----------------------------------------------------------------------===//
// Enumerations
//===----------------------------------------------------------------------===//
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
    SLOT_MENU_MANAGE_CC = 10 + mainMenuIdBase,
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

enum
{
    USE_NONE = 1,
    USE_PS,
    USE_SYNTH,
    USE_KONTAKT
};

enum
{
    HOST_PLAYING = 1,
    PLUGIN_QUERY_PLAY,
    PLUGIN_QUERY_PAUSE,
    PLUGIN_QUERY_STOP,
    PLUGIN_PLAYING,
    PLUGIN_STOPPED,
    PLUGIN_PAUSED
};

enum CommandIDs
{
    openPluginScanner = 1,
    openProject,
    saveProject,
    saveAsProject,
    exportMidi,
    importMidi,
    undoAction,
    redoAction
};

//==============================================================================
struct IdGenerator final
{
    using Id = int;

    static Id generateId()
    {
        return idCounter++;
    }

    static inline std::atomic<Id> idCounter{ 1 };
};

//==============================================================================
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

//==============================================================================
class ComponentWithTable : public Component, public TableListBoxModel
{
public:
    ComponentWithTable() {}
    ~ComponentWithTable() = default;

    void backgroundClicked(const MouseEvent&) override;

    virtual String getText(const int rowNumber, const int columnNumber) const { return ""; };
    virtual void setText(const int rowNumber, const int columnNumber, const String& newText) {};

    virtual int getSelectedId(const int rowNumber, const int columnNumber) const { return -1; };
    virtual void setSelectedId(const int rowNumber, const int columnNumber, const int newId) {};

protected:
    TableListBox table{ {}, this };
    Font font{ 14.0f };

    friend class EditableTextCustomComponent;
    friend class ComboBoxCustomComponent;
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

    void mouseDown(const MouseEvent& event) override;
    void setRowAndColumn(const int newRow, const int newColumn);
    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;
private:
    ComponentWithTable& owner;
    int rowId{ -1 }, columnId{ -1 };
};
