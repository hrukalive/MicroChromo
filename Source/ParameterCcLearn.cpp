#include "ParameterCcLearn.h"
#include "PluginBundle.h"

ParameterCcLearn::ProgressWindow::ProgressComponent::ProgressComponent(ProgressWindow& parent) : _parent(parent)
{
	textBlock.setReadOnly(true);
	textBlock.setMultiLine(true, true);
	textBlock.setCaretVisible(false);
	textBlock.setScrollbarsShown(true);
	textBlock.setFont(getLookAndFeel().getAlertWindowMessageFont());
	addAndMakeVisible(textBlock);

	btn.onClick = [this]() {
		_parent.closeButtonPressed();
	};
	addAndMakeVisible(btn);
}

void ParameterCcLearn::ProgressWindow::ProgressComponent::updateText(const String newText)
{
	MessageManagerLock lock;
	textBlock.setText(newText, false);
}

void ParameterCcLearn::ProgressWindow::ProgressComponent::paint(Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void ParameterCcLearn::ProgressWindow::ProgressComponent::resized()
{
	auto b = getLocalBounds();
	b.reduce(10, 10);
	btn.setBounds(b.removeFromBottom(24));
	b.removeFromBottom(4);
	textBlock.setBounds(b);
}

ParameterCcLearn::ProgressWindow::ProgressWindow(ParameterCcLearn& parent, Colour backgroundColor) :
	DocumentWindow("CC Learn", backgroundColor, 0),
	_parent(parent)
{
	component = std::make_unique<ProgressComponent>(*this);

	setSize(300, 200);

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

void ParameterCcLearn::ProgressWindow::updateText(const String newText)
{
	component->updateText(newText);
}

void ParameterCcLearn::ProgressWindow::closeButtonPressed()
{
	_parent.stopLearning();
	setVisible(false);
}

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

void ParameterCcLearn::processCc(int instanceIndex, int ccNumber, int ccValue, float sampleOffset)
{
	if (_isLearning)
	{
		tmpCcSource = ccNumber;
		progressWindow->updateText(updateProgressNote());
	}
	else if (hasLearned && ccSource == ccNumber)
		_bundle->getParameters(instanceIndex)[paramIndex]->setValueAt(ccValue / 127.0f * (learnedCcMax - learnedCcMin) + learnedCcMin, sampleOffset);
}

void ParameterCcLearn::reset()
{
	hasLearned = false;
	_isLearning = false;
	if (_parameters)
		for (auto* p : *_parameters)
			p->removeListener(this);
	_parameters = nullptr;
	resetTempValues();
	ccSource = -1;
	paramIndex = -1;
	learnedCcMin = FP_INFINITE;
	learnedCcMax = -FP_INFINITE;
	progressWindow->updateText(updateProgressNote());
}

void ParameterCcLearn::resetTempValues()
{
	tmpCcSource = -1;
	tmpParamIndex = -1;
	tmpLearnedCcMin = FP_INFINITE;
	tmpLearnedCcMax = -FP_INFINITE;
	progressWindow->updateText(updateProgressNote());
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
	if (validLearn())
	{
		if (hasLearned)
			(*_parameters)[paramIndex]->addListener(_bundle);

		paramIndex = tmpParamIndex.load();
		ccSource = tmpCcSource.load();
		learnedCcMin = tmpLearnedCcMin.load();
		learnedCcMax = tmpLearnedCcMax.load();
		resetTempValues();

		(*_parameters)[paramIndex]->removeListener(_bundle);
		hasLearned = true;

		AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::InfoIcon, "CC Learn", "CC Learn successful.");
	}
	else
		AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, "CC Learn", "CC Learn status not changed.");
}

void ParameterCcLearn::setCcLearn(int ccNumber, int parameterIndex, float min, float max)
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
	
	if (validLearn())
	{
		if (hasLearned)
			(*_parameters)[paramIndex]->addListener(_bundle);

		paramIndex = tmpParamIndex.load();
		ccSource = tmpCcSource.load();
		learnedCcMin = tmpLearnedCcMin.load();
		learnedCcMax = tmpLearnedCcMax.load();
		resetTempValues();

		(*_parameters)[paramIndex]->removeListener(_bundle);
		hasLearned = true;
	}
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
	}
}

void ParameterCcLearn::getStateInformation(MemoryBlock& destData)
{
	std::unique_ptr<XmlElement> xml = std::make_unique<XmlElement>("cclearn_module");
	xml->setAttribute("ccSource", ccSource);
	xml->setAttribute("paramIndex", paramIndex);
	xml->setAttribute("learnedCcMin", learnedCcMin);
	xml->setAttribute("learnedCcMax", learnedCcMax);
	AudioProcessor::copyXmlToBinary(*xml, destData);
}

void ParameterCcLearn::setStateInformation(const void* data, int sizeInBytes)
{
	std::unique_ptr<XmlElement> xml(AudioProcessor::getXmlFromBinary(data, sizeInBytes));
	if (xml.get() != nullptr)
	{
		if (xml->hasTagName("cclearn_module"))
		{
			setCcLearn(xml->getIntAttribute("ccSource", -1), 
				xml->getIntAttribute("paramIndex", -1), 
				xml->getDoubleAttribute("learnedCcMin", FP_INFINITE), 
				xml->getDoubleAttribute("learnedCcMax", -FP_INFINITE));
		}
	}
}

bool ParameterCcLearn::validLearn()
{
	return tmpParamIndex >= 0 && tmpParamIndex < (*_parameters).size()
		&& tmpCcSource >= 0 && tmpCcSource < 128
		&& tmpLearnedCcMin != FP_INFINITE && tmpLearnedCcMax != -FP_INFINITE
		&& tmpLearnedCcMin < tmpLearnedCcMax;
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
	if (!hasLearned)
		AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::InfoIcon, "CC Learn Status", "No learned parameter.");
	else
		AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::InfoIcon, "CC Learn Status",
			"CC Number: " + String(ccSource) + newLine.getDefault() +
			"Parameter: " + ((*_parameters)[paramIndex]->getName(256)) + " (" + String(paramIndex) + ")" + newLine.getDefault() +
			"Value min: " + String(learnedCcMin.load(), 3) + newLine.getDefault() +
			"Value max: " + String(learnedCcMax.load(), 3));
}