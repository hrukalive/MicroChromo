/*
  ==============================================================================

    Common.cpp
    Created: 25 Mar 2020 9:36:47pm
    Author:  bowen

  ==============================================================================
*/

#include "Common.h"
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

void ParameterLinker::linkParameter(AudioProcessorParameter* parameter)
{
    _genericParameter->setName(parameter->getName(128));
    dynamic_cast<AudioProcessorParameter*>(_genericParameter)->setValue(parameter->getValue());
    _parameter = parameter;
    listener1 = std::make_unique<Listener>(aLockValue, aLockGesture, aGestureOverride, bLockValue, bLockGesture, bGestureOverride, _genericParameter, _parameter);
    listener2 = std::make_unique<Listener>(bLockValue, bLockGesture, bGestureOverride, aLockValue, aLockGesture, aGestureOverride, _parameter, _genericParameter);
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

ParameterLinker::Listener::Listener(
    ABool& thisLockValue, ABool& thisLockGesture, ABool& thisGestureOverride,
    ABool& otherLockValue, ABool& otherLockGesture, ABool& otherGestureOverride,
    AudioProcessorParameter* paramSource, AudioProcessorParameter* paramDest)
    : _thisLockValue(thisLockValue), _thisLockGesture(thisLockGesture), _thisGestureOverride(thisGestureOverride), 
    _otherLockValue(otherLockValue), _otherLockGesture(otherLockGesture), _otherGestureOverride(otherGestureOverride), 
    _paramSource(paramSource), _paramDest(paramDest)
{
    _paramSource->addListener(this);
}
ParameterLinker::Listener::~Listener()
{
    _paramSource->removeListener(this);
}

void ParameterLinker::Listener::parameterValueChanged(int parameterIndex, float newValue)
{
    ignoreUnused(parameterIndex);
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
void ParameterLinker::Listener::parameterGestureChanged(int parameterIndex, bool gestureIsStarting)
{
    ignoreUnused(parameterIndex);
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
