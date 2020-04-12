#include "ParameterCcLearn.h"
#include "PluginProcessor.h"
#include "PluginBundle.h"

//==============================================================================
ParameterCcLearn::ProgressWindow::ProgressComponent::ProgressComponent(ProgressWindow& parent) : _parent(parent)
{
    textBlock.setReadOnly(true);
    textBlock.setMultiLine(true, true);
    textBlock.setCaretVisible(false);
    textBlock.setScrollbarsShown(true);
    textBlock.setFont(getLookAndFeel().getAlertWindowMessageFont());
    addAndMakeVisible(textBlock);

    ccNumberTextBox.setMultiLine(false, false);
    ccNumberTextBox.setTextToShowWhenEmpty("Enter or move CC", Colour::greyLevel(0.5));
    ccNumberTextBox.setCaretVisible(true);
    ccNumberTextBox.setInputRestrictions(3, "0123456789");
    ccNumberTextBox.onTextChange = [&]() {
        auto val = jlimit(0, 127, ccNumberTextBox.getText().getIntValue());
        _parent.getParent().processCc(-1, val, -1, -1);
        ccNumberTextBox.setText(String(val), false);
    };
    addAndMakeVisible(ccNumberTextBox);

    btn.onClick = [this]() {
        _parent.closeButtonPressed();
    };
    addAndMakeVisible(btn);
}

void ParameterCcLearn::ProgressWindow::ProgressComponent::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void ParameterCcLearn::ProgressWindow::ProgressComponent::resized()
{
    auto b = getLocalBounds();
    b.reduce(10, 10);
    ccNumberTextBox.setBounds(b.removeFromTop(24));
    btn.setBounds(b.removeFromBottom(24));
    b.removeFromBottom(4);
    textBlock.setBounds(b);
}

void ParameterCcLearn::ProgressWindow::ProgressComponent::updateText(const String& newText)
{
    MessageManagerLock lock;
    textBlock.setText(newText, false);
}

void ParameterCcLearn::ProgressWindow::ProgressComponent::updateCc(int ccNumber)
{
    MessageManagerLock lock;
    if (ccNumber == -1)
        ccNumberTextBox.setText("", false);
    else
        ccNumberTextBox.setText(String(ccNumber), false);
}


//==============================================================================
ParameterCcLearn::ProgressWindow::ProgressWindow(ParameterCcLearn& parent, Colour backgroundColor) :
    DocumentWindow("CC Learn", backgroundColor, 0),
    _parent(parent)
{
    component = std::make_unique<ProgressComponent>(*this);

    setSize(300, 230);

    setUsingNativeTitleBar(false);
    setAlwaysOnTop(true);
    setContentOwned(component.get(), false);
    centreWithSize(getWidth(), getHeight());
    component->setSize(getWidth(), getHeight());
    setVisible(false);
}

ParameterCcLearn::ProgressWindow::~ProgressWindow()
{
    component = nullptr;
}

void ParameterCcLearn::ProgressWindow::updateText(const String& newText)
{
    component->updateText(newText);
}

void ParameterCcLearn::ProgressWindow::updateCc(int ccNumber)
{
    component->updateCc(ccNumber);
}

void ParameterCcLearn::ProgressWindow::closeButtonPressed()
{
    _parent.stopLearning();
    setVisible(false);
}







//==============================================================================
ParameterCcLearn::StatusComponent::StatusComponent(ParameterCcLearn& parent) : _parent(parent), items(parent.items)
{
    table.getHeader().addColumn("Parameter", 1, 200, 200, 300, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("Min", 2, 60, 60, 120, TableHeaderComponent::notSortable);
    table.getHeader().addColumn("Max", 3, 60, 60, 120, TableHeaderComponent::notSortable);
    table.setColour(ListBox::outlineColourId, Colours::grey);
    table.setOutlineThickness(1);
    table.setMultipleSelectionEnabled(false);
    addAndMakeVisible(table);

    ccNumberTextBox.setMultiLine(false, false);
    ccNumberTextBox.setTextToShowWhenEmpty("Enter or move CC", Colour::greyLevel(0.5));
    ccNumberTextBox.setCaretVisible(true);
    ccNumberTextBox.setInputRestrictions(3, "0123456789");
    ccNumberTextBox.onTextChange = [&]() {
        auto val = jlimit(0, 127, ccNumberTextBox.getText().getIntValue());
        ccNumberTextBox.setText(String(val), false);
    };
    if (parent.ccSource > -1)
        ccNumberTextBox.setText(String(parent.ccSource), false);
    addAndMakeVisible(ccNumberTextBox);

    setBtn.onClick = [&]() {
        _parent.setCcSource(ccNumberTextBox.getText().getIntValue());
    };
    addAndMakeVisible(setBtn);

    removeBtn.onClick = [&]() {
        if (lastRow > -1)
        {
            _parent.removeCcLearn(items[lastRow].paramIndex);
            table.updateContent();
        }
    };
    addAndMakeVisible(removeBtn);

    doneBtn.onClick = [&]() {
        this->getTopLevelComponent()->exitModalState(0);
    };
    addAndMakeVisible(doneBtn);

    table.updateContent();

    setSize(350, 300);
}

void ParameterCcLearn::StatusComponent::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void ParameterCcLearn::StatusComponent::resized()
{
    auto b = getLocalBounds();
    b.reduce(10, 10);

    auto tmp = b.removeFromTop(24);
    setBtn.setBounds(tmp.removeFromRight(50));
    tmp.removeFromRight(6);
    ccNumberTextBox.setBounds(tmp);
    b.removeFromTop(6);

    tmp = b.removeFromBottom(24);
    removeBtn.setBounds(tmp.removeFromRight(100));
    tmp.removeFromRight(6);
    doneBtn.setBounds(tmp);

    b.removeFromBottom(6);

    table.setBounds(b);
}

int ParameterCcLearn::StatusComponent::getNumRows()
{
    return items.size();
}

void ParameterCcLearn::StatusComponent::paintRowBackground(Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected)
{
    auto alternateColour = getLookAndFeel().findColour(ListBox::backgroundColourId)
        .interpolatedWith(getLookAndFeel().findColour(ListBox::textColourId), 0.03f);
    if (rowIsSelected)
        g.fillAll(findColour(TextEditor::highlightColourId));
    else if (rowNumber % 2)
        g.fillAll(alternateColour);
}

void ParameterCcLearn::StatusComponent::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    g.setColour(getLookAndFeel().findColour(ListBox::textColourId));
    g.setFont(font);

    String text;
    switch (columnId)
    {
    case 1: text = items[rowNumber].paramName + " (" + String(items[rowNumber].paramIndex) + ")"; break;
    case 2: text = String(items[rowNumber].learnedCcMin, 3); break;
    case 3: text = String(items[rowNumber].learnedCcMax, 3); break;
    default:
        break;
    }
    g.drawText(text, 2, 0, width - 4, height, Justification::centredLeft, true);

    g.setColour(getLookAndFeel().findColour(ListBox::backgroundColourId));
    g.fillRect(width - 1, 0, 1, height);
}

Component* ParameterCcLearn::StatusComponent::refreshComponentForCell(int rowNumber, int columnId, bool /*isRowSelected*/, Component* existingComponentToUpdate)
{
    if (columnId == 2 || columnId == 3)
    {
        auto* textLabel = static_cast<EditableTextCustomComponent*> (existingComponentToUpdate);
        if (textLabel == nullptr)
            textLabel = new EditableTextCustomComponent(*this);
        textLabel->setRowAndColumn(rowNumber, columnId);
        return textLabel;
    }

    jassert(existingComponentToUpdate == nullptr);
    return nullptr;
}

int ParameterCcLearn::StatusComponent::getColumnAutoSizeWidth(int columnId)
{
    int widest = 92;

    for (auto i = getNumRows(); --i >= 0;)
    {

        String text;
        switch (columnId)
        {
        case 1: text = items[i].paramName + " (" + String(items[i].paramIndex) + ")"; break;
        case 2: text = String(items[i].learnedCcMin, 3); break;
        case 3: text = String(items[i].learnedCcMax, 3); break;
        default:
            break;
        }
        widest = jmax(widest, font.getStringWidth(text));
    }

    return widest + 8;
}

String ParameterCcLearn::StatusComponent::getText(const int rowNumber, const int columnNumber) const
{
    String text;
    switch (columnNumber)
    {
    case 2: text = String(items[rowNumber].learnedCcMin, 3); break;
    case 3: text = String(items[rowNumber].learnedCcMax, 3); break;
    default:
        break;
    }
    return text;
}
void ParameterCcLearn::StatusComponent::setText(const int rowNumber, const int columnNumber, const String& newText)
{
    switch (columnNumber)
    {
    case 2: items.getReference(rowNumber).learnedCcMin = newText.getFloatValue(); break;
    case 3: items.getReference(rowNumber).learnedCcMax = newText.getFloatValue(); break;
    default:
        break;
    }
}





//==============================================================================
ParameterCcLearn::ParameterCcLearn(PluginBundle* bundle) :
    _bundle(bundle)
{
    progressWindow = std::make_unique<ProgressWindow>(*this, LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

ParameterCcLearn::~ParameterCcLearn()
{
    progressWindow = nullptr;
    if (_parameters)
        for (auto* p : *_parameters)
            p->removeListener(this);
}

void ParameterCcLearn::processCc(int instanceIndex, int ccNumber, int ccValue, float /*sampleOffset*/)
{
    if (_isLearning)
    {
        tmpCcSource = ccNumber;
        progressWindow->updateText(updateProgressNote());
        progressWindow->updateCc(tmpCcSource);
    }
    else if (_hasLearned && ccSource == ccNumber)
        for (auto& item : items)
            _bundle->getParameters(instanceIndex)[item.paramIndex]->setValue(ccValue / 100.0f * (item.learnedCcMax - item.learnedCcMin) + item.learnedCcMin);
}

bool ParameterCcLearn::validLearn()
{
    return tmpParamIndex >= 0 && tmpParamIndex < (*_parameters).size()
        && tmpCcSource >= 0 && tmpCcSource < 128
        && tmpLearnedCcMin != FP_INFINITE && tmpLearnedCcMax != -FP_INFINITE
        && tmpLearnedCcMin < tmpLearnedCcMax;
}

void ParameterCcLearn::addCcLearn(int ccNumber, int parameterIndex, float min, float max)
{
    _isLearning = false;
    if (_parameters)
        for (auto* p : *_parameters)
            p->removeListener(this);

    tmpCcSource = ccNumber;
    tmpParamIndex = parameterIndex;
    tmpLearnedCcMin = min;
    tmpLearnedCcMax = max;
    _parameters = &(_bundle->getParameters());

    if (validLearn() && !learnedIndices.contains(tmpParamIndex))
    {
        ccSource = tmpCcSource.load();
        CcItem item;
        item.paramIndex = tmpParamIndex.load();
        item.paramName = (*_parameters)[item.paramIndex]->getName(128);
        item.learnedCcMin = tmpLearnedCcMin.load();
        item.learnedCcMax = tmpLearnedCcMax.load();
        items.add(item);
        items.sort(CcItemComparator());
        learnedIndices.insert(tmpParamIndex);
        resetTempValues();

        (*_parameters)[item.paramIndex]->removeListener(_bundle);
        _hasLearned = true;

        _bundle->getProcessor().updatePitchShiftModulationSource();
    }
}

void ParameterCcLearn::removeCcLearn(int parameterIndex)
{
    if (!learnedIndices.contains(parameterIndex))
        return;
    if (learnedIndices.size() == 1)
    {
        reset(true);
        return;
    }
    if (_parameters)
        (*_parameters)[parameterIndex]->addListener(_bundle);
    items.removeIf([parameterIndex](CcItem item) { return item.paramIndex == parameterIndex; });
    learnedIndices.erase(parameterIndex);
}

void ParameterCcLearn::resetTempValues()
{
    tmpCcSource = ccSource.load();
    tmpParamIndex = -1;
    tmpLearnedCcMin = FP_INFINITE;
    tmpLearnedCcMax = -FP_INFINITE;
    progressWindow->updateText(updateProgressNote());
    progressWindow->updateCc(tmpCcSource);
}

void ParameterCcLearn::setCcSource(int newCcSource)
{
    if (newCcSource >= 0 && newCcSource < 128)
    {
        ccSource = newCcSource;
        _bundle->getProcessor().updatePitchShiftModulationSource();
    }
}

void ParameterCcLearn::reset(bool notify)
{
    _hasLearned = false;
    _isLearning = false;
    if (_parameters)
    {
        for (auto* p : *_parameters)
            p->removeListener(this);
        for (auto& item : items)
            (*_parameters)[item.paramIndex]->addListener(_bundle);
    }
    _parameters = nullptr;
    ccSource = -1;
    items.clear();
    learnedIndices.clear();
    resetTempValues();
    progressWindow->updateText(updateProgressNote());
    progressWindow->updateCc(tmpCcSource);

    if (notify)
        _bundle->getProcessor().updatePitchShiftModulationSource();
}

void ParameterCcLearn::startLearning()
{
    resetTempValues();
    _isLearning = true;
    _parameters = &(_bundle->getParameters());
    for (auto* p : *_parameters)
        p->addListener(this);

    progressWindow->setVisible(true);
}

void ParameterCcLearn::stopLearning()
{
    if (_isLearning)
    {
        _isLearning = false;
        for (auto* p : *_parameters)
            p->removeListener(this);
    }
    if (validLearn() && !learnedIndices.contains(tmpParamIndex))
    {
        ccSource = tmpCcSource.load();
        CcItem item;
        item.paramIndex = tmpParamIndex.load();
        item.paramName = (*_parameters)[item.paramIndex]->getName(128);
        item.learnedCcMin = tmpLearnedCcMin.load();
        item.learnedCcMax = tmpLearnedCcMax.load();
        items.add(item);
        items.sort(CcItemComparator());
        learnedIndices.insert(tmpParamIndex);
        resetTempValues();

        (*_parameters)[item.paramIndex]->removeListener(_bundle);
        _hasLearned = true;

        _bundle->getProcessor().updatePitchShiftModulationSource();

        AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::InfoIcon, "CC Learn", "CC Learn successful.");
    }
    else
        AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, "CC Learn", "CC Learn status not changed.");
}

void ParameterCcLearn::parameterValueChanged(int parameterIndex, float newValue)
{
    if (_isLearning)
    {
        if (tmpParamIndex != parameterIndex)
        {
            if (_bundle->isParameterExposed(parameterIndex))
            {
                progressWindow->updateText(updateProgressNote("Parameter moved cannot be learned because it is exposed to host."));
                progressWindow->updateCc(tmpCcSource);
                return;
            }
            if (learnedIndices.contains(parameterIndex))
            {
                progressWindow->updateText(updateProgressNote("Parameter moved has already been learned."));
                progressWindow->updateCc(tmpCcSource);
                return;
            }
            tmpLearnedCcMin = FP_INFINITE;
            tmpLearnedCcMax = -FP_INFINITE;
        }
        tmpParamIndex = parameterIndex;
        if (newValue > tmpLearnedCcMax)
            tmpLearnedCcMax = newValue;
        else if (newValue < tmpLearnedCcMin)
            tmpLearnedCcMin = newValue;
        progressWindow->updateText(updateProgressNote());
        progressWindow->updateCc(tmpCcSource);
    }
}

std::unique_ptr<XmlElement> ParameterCcLearn::createXml()
{
    std::unique_ptr<XmlElement> xml = std::make_unique<XmlElement>("ccLearnModule");
    xml->setAttribute("ccSource", ccSource);

    items.sort(CcItemComparator());
    for (auto& item : items)
    {
        auto* itemsXml = xml->createNewChildElement("item");
        itemsXml->setAttribute("paramIndex", item.paramIndex);
        itemsXml->setAttribute("learnedCcMin", item.learnedCcMin);
        itemsXml->setAttribute("learnedCcMax", item.learnedCcMax);
    }
    return xml;
}

void ParameterCcLearn::loadFromXml(const XmlElement* xml)
{
    if (xml != nullptr && xml->getTagName() == "ccLearnModule")
    {
        auto src = xml->getIntAttribute("ccSource", -1);
        if (src > -1)
        {
            forEachXmlChildElementWithTagName(*xml, child, "item")
            {
                addCcLearn(src,
                    child->getIntAttribute("paramIndex", -1),
                    child->getDoubleAttribute("learnedCcMin", FP_INFINITE),
                    child->getDoubleAttribute("learnedCcMax", -FP_INFINITE));
            }
            items.sort(CcItemComparator());
        }
    }
}

String ParameterCcLearn::updateProgressNote(String otherMsg)
{
    String progressNote;
    if (tmpCcSource >= 0 && tmpCcSource < 128)
        progressNote.append("CC Number: " + String(tmpCcSource) + newLine.getDefault(), 256);
    else
        progressNote.append("CC Number: ?" + String(newLine.getDefault()), 256);
    if (tmpParamIndex >= 0 && tmpParamIndex < (*_parameters).size())
        progressNote.append("Parameter: " + ((*_parameters)[tmpParamIndex]->getName(256)) + " (" + String(tmpParamIndex) + ")" + newLine.getDefault(), 256);
    else
        progressNote.append("Parameter: ?" + String(newLine.getDefault()), 256);
    if (tmpLearnedCcMin != FP_INFINITE)
        progressNote.append("Value min: " + String(tmpLearnedCcMin.load(), 3) + newLine.getDefault(), 256);
    else
        progressNote.append("Value min: ?" + String(newLine.getDefault()), 256);
    if (tmpLearnedCcMax != -FP_INFINITE)
        progressNote.append("Value max: " + String(tmpLearnedCcMax.load(), 3) + newLine.getDefault(), 256);
    else
        progressNote.append("Value max: ?" + String(newLine.getDefault()), 256);
    progressNote.append(otherMsg, 256);
    return progressNote;
}

void ParameterCcLearn::showStatus()
{
    if (!_hasLearned)
        AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::InfoIcon, "CC Learn Status", "No learned parameter.");
    else
    {
        DialogWindow::LaunchOptions dialogOption;

        dialogOption.dialogTitle = "CC Status Manager";
        dialogOption.dialogBackgroundColour = LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId);
        dialogOption.escapeKeyTriggersCloseButton = false;
        dialogOption.useNativeTitleBar = false;
        dialogOption.resizable = true;
        dialogOption.content.setOwned(new StatusComponent(*this));
        dialogOption.launchAsync();
    }
}
