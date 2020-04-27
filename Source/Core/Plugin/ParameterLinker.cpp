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

#include "ParameterLinker.h"

#include "PluginBundle.h"

//__________________________________________________________________________
//                                                                          |\
// ParameterLinker                                                          | |
//__________________________________________________________________________| |
//___________________________________________________________________________\|

ParameterLinker::ParameterLinker(AudioParameterFloatVariant* genericParameter, std::shared_ptr<PluginBundle>& bundle)
    : _genericParameter(genericParameter), _bundle(bundle)
{}
ParameterLinker::~ParameterLinker()
{
    resetLink();
    _genericParameter = nullptr;
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

//__________________________________________________________________________
//                                                                          |\
// OneToOneListener (Main instance -> Host)                                 | |
//__________________________________________________________________________| |
//___________________________________________________________________________\|

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

//===------------------------------------------------------------------===//
// Listeners
//===------------------------------------------------------------------===//
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

//__________________________________________________________________________
//                                                                          |\
// OneToManyListener (Host -> Multiple instances)                           | |
//__________________________________________________________________________| |
//___________________________________________________________________________\|

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

//===------------------------------------------------------------------===//
// Listeners
//===------------------------------------------------------------------===//
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
