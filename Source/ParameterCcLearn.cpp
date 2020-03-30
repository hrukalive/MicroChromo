#include "ParameterCcLearn.h"
#include "PluginBundle.h"

ParameterCcLearn::ParameterCcLearn(PluginBundle* bundle) :
	_bundle(bundle) {}

ParameterCcLearn::~ParameterCcLearn()
{
	if (_parameters)
		for (auto* p : *_parameters)
			p->removeListener(this);
}

void ParameterCcLearn::processCc(int instanceIndex, int ccNumber, int ccValue, float sampleOffset)
{
	if (isLearning)
		tmpCcSource = ccNumber;
	else if (hasLearned && ccSource == ccNumber)
		_bundle->getParameters(instanceIndex)[paramIndex]->setValueAt(ccValue / 127.0f * (learnedCcMax - learnedCcMin) + learnedCcMin, sampleOffset);
}

void ParameterCcLearn::reset()
{
	hasLearned = false;
	isLearning = false;
	if (_parameters)
		for (auto* p : *_parameters)
			p->removeListener(this);
	_parameters = nullptr;
	resetTempValues();
	ccSource = -1;
	paramIndex = -1;
	learnedCcMin = FP_INFINITE;
	learnedCcMax = -FP_INFINITE;
}

void ParameterCcLearn::resetTempValues()
{
	tmpCcSource = -1;
	tmpParamIndex = -1;
	tmpLearnedCcMin = FP_INFINITE;
	tmpLearnedCcMax = -FP_INFINITE;
}

void ParameterCcLearn::startLearning()
{
	resetTempValues();
	isLearning = true;
	_parameters = &(_bundle->getParameters());
	for (auto* p : *_parameters)
		p->addListener(this);
}

void ParameterCcLearn::stopLearning()
{
	if (isLearning)
	{
		isLearning = false;
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
	}
}

void ParameterCcLearn::setCcLearn(int ccNumber, int parameterIndex, float min, float max)
{
	isLearning = true;
	tmpCcSource = ccNumber;
	tmpParamIndex = parameterIndex;
	tmpLearnedCcMin = min;
	tmpLearnedCcMax = max;
	_parameters = &(_bundle->getParameters());
	stopLearning();
}

void ParameterCcLearn::parameterValueChanged(int parameterIndex, float newValue)
{
	if (isLearning)
	{
		if (tmpParamIndex != parameterIndex)
		{
			if (_bundle->isParameterExposed(parameterIndex))
			{
				if (!alertBoxShowing)
				{
					alertBoxShowing = true;
					AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "Cannot CC Learn", "This parameter is exposed to host.", "OK");
					alertBoxShowing = false;
				}
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
			tmpCcSource = xml->getIntAttribute("ccSource", -1);
			tmpParamIndex = xml->getIntAttribute("paramIndex", -1);
			tmpLearnedCcMin = xml->getDoubleAttribute("learnedCcMin", FP_INFINITE);
			tmpLearnedCcMax = xml->getDoubleAttribute("learnedCcMax", -FP_INFINITE);
			_parameters = &(_bundle->getParameters());
			stopLearning();
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
