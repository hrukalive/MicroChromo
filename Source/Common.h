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

    PLUGIN_SORT_MANUFACTURER = 14 + mainMenuIdBase,
    PLUGIN_SORT_CATEGORY = 15 + mainMenuIdBase,
    PLUGIN_SORT_ALPHABETICALLY = 16 + mainMenuIdBase,
    PLUGIN_SORT_FORMAT = 17 + mainMenuIdBase
};

enum CommandIDs
{
    openPluginScanner = 1,
    openMidiScanner,
    testCommand
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
