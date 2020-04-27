/*
  ==============================================================================

    ParameterLinker.h
    Created: 24 Apr 2020 11:04:08pm
    Author:  bowen

  ==============================================================================
*/

#pragma once

#include "Common.h"

class PluginBundle;

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
