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
class ParameterCcLearn : public AudioProcessorParameter::Listener
{
public:
    ParameterCcLearn(PluginBundle* bundle);
    ~ParameterCcLearn();

    void processCc(int instanceIndex, int ccNumber, int ccValue, float sampleOffset);
    void addCcLearn(int ccNumber, int parameterIndex, float min, float max);
    void removeCcLearn(int parameterIndex);
    void resetCcLearn(bool notify = false);
    void startLearning();

    //===------------------------------------------------------------------===//
    // Listeners
    //===------------------------------------------------------------------===//
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}

    //===------------------------------------------------------------------===//
    // Accessors
    //===------------------------------------------------------------------===//
    auto& getCcLearnedParameterIndex() { return learnedIndices; }
    int getCcSource() { return ccSource; }
    bool isLearning() { return _isLearning; }
    bool hasLearned() { return _hasLearned; }

    //===------------------------------------------------------------------===//
    // Helpers
    //===------------------------------------------------------------------===//
    void showStatus();

    //===------------------------------------------------------------------===//
    // Serialzation
    //===------------------------------------------------------------------===//
    std::unique_ptr<XmlElement> createXml();
    void loadFromXml(const XmlElement* xml);

private:
    //===------------------------------------------------------------------===//
    // Helpers
    //===------------------------------------------------------------------===//
    void resetTempValues();
    void setCcSource(int newCcSource);
    bool validLearn();
    void stopLearning();

    String updateProgressNote(String otherMsg = "");

    //==============================================================================
    struct CcItem
    {
        String paramName = "";
        int paramIndex{ -1 };
        float learnedCcMin{ FLT_MAX }, learnedCcMax{ -FLT_MAX };
    };

    struct CcItemComparator
    {
        CcItemComparator() = default;

        static int compareElements(const CcItem& first, const CcItem& second)
        {
            const auto valDiff = first.paramIndex - second.paramIndex;
            return (valDiff > 0.f) - (valDiff < 0.f);
        }
    };

    //==============================================================================
    PluginBundle* _bundle;
    const Array<AudioProcessorParameter*>* _parameters{ nullptr };
    std::atomic<int> ccSource{ -1 };
    std::atomic<bool> _isLearning = false, _hasLearned = false;

    Array<CcItem> items;
    std::unordered_set<int> learnedIndices;

    std::atomic<int> tmpParamIndex{ -1 }, tmpCcSource{ -1 };
    std::atomic<float> tmpLearnedCcMin{ FLT_MAX }, tmpLearnedCcMax{ -FLT_MAX };

    //==============================================================================
    class ProgressWindow : public DocumentWindow
    {
    public:
        ProgressWindow(ParameterCcLearn& parent, Colour backgroundColor);
        ~ProgressWindow();

        //===------------------------------------------------------------------===//
        // Accessors
        //===------------------------------------------------------------------===//
        ParameterCcLearn& getParent() { return _parent; }

        //===------------------------------------------------------------------===//
        // Helpers
        //===------------------------------------------------------------------===//
        void updateText(const String& newText);
        void updateCc(int ccNumber);
        void closeButtonPressed() override;

    private:
        ParameterCcLearn& _parent;

        class ProgressComponent : public Component
        {
        public:
            ProgressComponent(ProgressWindow& parent);
            ~ProgressComponent() = default;

            //===------------------------------------------------------------------===//
            // Component
            //===------------------------------------------------------------------===//
            void paint(Graphics& g) override;
            void resized() override;

            //===------------------------------------------------------------------===//
            // Helpers
            //===------------------------------------------------------------------===//
            void updateText(const String& newText);
            void updateCc(int ccNumber);

        private:
            ProgressWindow& _parent;

            String text;
            TextEditor textBlock, ccNumberTextBox;
            TextButton btn{ "Done" };

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgressComponent)
        };
        std::unique_ptr<ProgressComponent> component;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgressWindow)
    };
    std::unique_ptr<ProgressWindow> progressWindow;


    class StatusComponent : public ComponentWithTable
    {
    public:
        StatusComponent(ParameterCcLearn& parent);
        ~StatusComponent() = default;

        //===------------------------------------------------------------------===//
        // Component
        //===------------------------------------------------------------------===//
        void paint(Graphics& g) override;
        void resized() override;

        //===------------------------------------------------------------------===//
        // TableListModel
        //===------------------------------------------------------------------===//
        int getNumRows() override;

        void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
        void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
        Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override;

        int getColumnAutoSizeWidth(int columnId) override;

        //===------------------------------------------------------------------===//
        // ComponentWithTable
        //===------------------------------------------------------------------===//
        String getText(const int rowNumber, const int columnNumber) const override;
        void setText(const int rowNumber, const int columnNumber, const String& newText) override;

    private:
        ParameterCcLearn& _parent;
        Array<CcItem>& items;

        TextEditor ccNumberTextBox;
        TextButton removeBtn{ "Remove" }, setBtn{ "Set" }, doneBtn{ "Done" };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatusComponent)
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterCcLearn)
};
