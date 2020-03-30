#pragma once

#include <JuceHeader.h>

class PluginBundle;

class ParameterCcLearn : public AudioProcessorParameter::Listener
{
public:
	ParameterCcLearn(PluginBundle* bundle);
	~ParameterCcLearn();

	void processCc(int instanceIndex, int ccNumber, int ccValue, float sampleOffset);
	void reset();
	void startLearning();
	void stopLearning();
	void setCcLearn(int ccNumber, int parameterIndex, float min, float max);

	void parameterValueChanged(int parameterIndex, float newValue) override;
	void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}

	void getStateInformation(MemoryBlock& destData);
	void setStateInformation(const void* data, int sizeInBytes);

	int getCcLearnedParameterIndex() { return paramIndex; }

private:
	void resetTempValues();
	bool validLearn();

	PluginBundle* _bundle;
	const Array<AudioProcessorParameter*>* _parameters{ nullptr };
	std::atomic<int> paramIndex{ -1 }, ccSource{ -1 }, tmpParamIndex{ -1 }, tmpCcSource{ -1 };
	std::atomic<float> learnedCcMin{ FP_INFINITE }, learnedCcMax{ -FP_INFINITE }, tmpLearnedCcMin{ FP_INFINITE }, tmpLearnedCcMax{ -FP_INFINITE };
	std::atomic<bool> isLearning = false, hasLearned = false, alertBoxShowing = false;
};
