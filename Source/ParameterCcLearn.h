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
	void setCcLearn(int ccNumber, int parameterIndex, float min, float max);

	void parameterValueChanged(int parameterIndex, float newValue) override;
	void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}

	void getStateInformation(MemoryBlock& destData);
	void setStateInformation(const void* data, int sizeInBytes);

	int getCcLearnedParameterIndex() { return paramIndex; }
	bool isLearning() { return _isLearning; }
	void showStatus();

private:

	class ProgressWindow : public DocumentWindow
	{
	public:
		ProgressWindow(ParameterCcLearn& parent, Colour backgroundColor);
		~ProgressWindow();

		void updateText(String newText);
		void closeButtonPressed() override;

	private:
		class ProgressComponent : public Component
		{
		public:
			ProgressComponent(ProgressWindow& parent);
			~ProgressComponent() = default;

			//==============================================================================
			void paint(Graphics& g) override;
			void resized() override;

			void updateText(String newText);

		private:
			String text;
			TextEditor textBlock;
			TextButton btn{ "Done" };
			ProgressWindow& _parent;

			JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgressComponent)
		};

		std::unique_ptr<ProgressComponent> component;
		ParameterCcLearn& _parent;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgressWindow)
	};

	void resetTempValues();
	bool validLearn();
	void stopLearning();

	String updateProgressNote(String otherMsg = "");

	PluginBundle* _bundle;
	const Array<AudioProcessorParameter*>* _parameters{ nullptr };
	std::unique_ptr<ProgressWindow> progressWindow;
	std::atomic<int> paramIndex{ -1 }, ccSource{ -1 }, tmpParamIndex{ -1 }, tmpCcSource{ -1 };
	std::atomic<float> learnedCcMin{ FP_INFINITE }, learnedCcMax{ -FP_INFINITE }, tmpLearnedCcMin{ FP_INFINITE }, tmpLearnedCcMax{ -FP_INFINITE };
	std::atomic<bool> _isLearning = false, hasLearned = false, alertBoxShowing = false;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterCcLearn)
};
