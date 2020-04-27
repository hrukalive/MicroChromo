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

#include "Common.h"

class PluginBundle;

//==============================================================================
class ParameterLinkEditor  : public Component, public ListBoxModel
{
public:
    ParameterLinkEditor (PluginBundle& bundle);
    ~ParameterLinkEditor() = default;

    //===------------------------------------------------------------------===//
    // Components
    //===------------------------------------------------------------------===//
    void paint(Graphics& g) override;
    void resized() override;

    //===------------------------------------------------------------------===//
    // ListBoxModel
    //===------------------------------------------------------------------===//
    int getNumRows() override { return parameters.size(); }
    void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const MouseEvent& e) override;

private:
    //==============================================================================
    PluginBundle& _bundle;
    Array<std::pair<AudioProcessorParameter*, int>> parameters;

    const int numParameterSlot;
    std::unordered_set<int> selectedSet;

    //==============================================================================
    ListBox parameterList;
    Label statusLabel;
    TextButton okBtn{ "OK" };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterLinkEditor)
};
