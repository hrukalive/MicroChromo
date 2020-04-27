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

#include <JuceHeader.h>

//==============================================================================
namespace Serialization
{
    namespace Core
    {
        static const Identifier treeRoot = "tree";
        static const Identifier treeNode = "node";
        static const Identifier treeNodeType = "type";
        static const Identifier treeNodeName = "name";
        static const Identifier treeState = "treeState";
        static const Identifier selectedTreeNode = "selectedNode";
        static const Identifier treeNodeId = "nodeId";

        static const Identifier workspace = "workspace";
        static const Identifier root = "root";
        static const Identifier settings = "settings";
        static const Identifier instrumentsList = "instruments";
        static const Identifier instrumentRoot = "instrument";

        static const Identifier project = "project";
        static const Identifier projectId = "projectId";
        static const Identifier projectName = "projectName";
        static const Identifier projectInfo = "projectInfo";
        static const Identifier projectTimeStamp = "projectTimeStamp";
        static const Identifier versionControl = "versionControl";
        static const Identifier patternSet = "patternSet";
        static const Identifier trackGroup = "group";
        static const Identifier track = "track";
        static const Identifier pianoTrack = "pianoTrack";
        static const Identifier automationTrack = "automationTrack";
        static const Identifier projectTimeline = "projectTimeline";
        static const Identifier filePath = "filePath";

        // Properties
        static const Identifier trackId = "trackId";
        static const Identifier trackName = "name";
        static const Identifier trackChannel = "channel";

        // Timeline
        static const Identifier annotationsTrackId = "annotationsTrackId";
        static const Identifier keySignaturesTrackId = "keySignaturesTrackId";
        static const Identifier timeSignaturesTrackId = "timeSignaturesTrackId";

        static const Identifier globalConfig = "config";

        // CcLearn
        static const Identifier ccLearn = "ccLearnModule";
        static const Identifier ccSource = "ccSource";
        static const Identifier ccItem = "ccItem";
        static const Identifier ccParamIndex = "ccParamIndex";
        static const Identifier ccLearnedCcMin = "ccLearnedCcMin";
        static const Identifier ccLearnedCcMax = "ccLearnedCcMax";

        // Processor
        static const Identifier processor = "processor";

    } // namespace Core

    namespace Midi
    {
        // Tracks
        static const Identifier track = "track";
        static const Identifier notes = "notes";
        static const Identifier timeSignatures = "timeSignatures";
        static const Identifier tempoMarkers = "tempoMarkers";

        // Events
        static const Identifier note = "note";
        static const Identifier timeSignature = "timeSignature";
        static const Identifier tempoMarker = "tempoMarker";

        // Properties

        // notes are the most common records in the savefile
        // therefore their properties are shortened
        static const Identifier id = "id";
        static const Identifier key = "key";
        static const Identifier timestamp = "ts";
        static const Identifier length = "len";
        static const Identifier volume = "vol";
        static const Identifier pitchColor = "pcolor";

        static const Identifier numerator = "numerator";
        static const Identifier denominator = "denominator";

        static const Identifier bpm = "bpm";
    } // namespace Events

    namespace PitchColor
    {
        static const Identifier colorMapPresets = "pitchColorMapPresets";
        static const Identifier colorMap = "pitchColorMap";
        static const Identifier entry = "entry";
        static const Identifier name = "name";
        static const Identifier value = "value";
        static const Identifier color = "color";
        static const Identifier defaultKeys = "defaultKeys";
    } // namespace PitchColor

    namespace Config
    {
        static const Identifier currentLocale = "currentLocale";
    } // namespace Config

    namespace UI
    {
        static const Identifier sequencer = "sequencer";

        static const Identifier pianoRoll = "pianoRoll";
        static const Identifier patternRoll = "patternRoll";

        static const Identifier startBar = "startBar";
        static const Identifier endBar = "endBar";
        static const Identifier barWidth = "barWidth";
        static const Identifier rowHeight = "rowHeight";
        static const Identifier viewportPositionX = "viewportX";
        static const Identifier viewportPositionY = "viewportY";

        static const Identifier positionX = "positionX";
        static const Identifier positionY = "positionY";

        namespace Hotkeys
        {
            static const Identifier scheme = "hotkeyScheme";
            static const Identifier schemeName = "name";
            static const Identifier keyPress = "keyPress";
            static const Identifier keyDown = "keyDown";
            static const Identifier keyUp = "keyUp";
            static const Identifier hotkeyDescription = "key";
            static const Identifier hotkeyReceiver = "receiver";
            static const Identifier hotkeyCommand = "command";
        }

        namespace Colours
        {
            static const Identifier scheme = "colourScheme";
            static const Identifier colourMap = "colourMap";
            static const Identifier name = "name";

            static const Identifier primaryGradientA = "primaryGradientA";
            static const Identifier primaryGradientB = "primaryGradientB";
            static const Identifier secondaryGradientA = "secondaryGradientA";
            static const Identifier secondaryGradientB = "secondaryGradientB";

            static const Identifier panelFill = "panelFill";
            static const Identifier panelBorder = "panelBorder";

            static const Identifier lassoFill = "lassoFill";
            static const Identifier lassoBorder = "lassoBorder";

            static const Identifier blackKey = "blackKey";
            static const Identifier whiteKey = "whiteKey";

            static const Identifier row = "row";
            static const Identifier beat = "beat";
            static const Identifier bar = "bar";

            static const Identifier text = "text";

            static const Identifier iconBase = "iconBase";
            static const Identifier iconShadow = "iconShadow";
        } // namespace Colours
        
    } // namespace UI

    namespace Translations
    {
        static const Identifier metaSymbol = "{x}";

        static const Identifier wrapperClassName = "pluralForm";
        static const Identifier wrapperMethodName = "detect";

        static const Identifier translation = "translation";
        static const Identifier locale = "locale";
        static const Identifier literal = "literal";
        static const Identifier author = "author";
        static const Identifier name = "name";
        static const Identifier id = "id";

        static const Identifier pluralEquation = "pluralEquation";
        static const Identifier pluralLiteral = "pluralLiteral";
        static const Identifier pluralForm = "pluralForm";
    }  // namespace Translations
}  // namespace Serialization
