#pragma once

#include <JuceHeader.h>

class PluginBundle;

//==============================================================================
class ParameterCcLearn : public AudioProcessorParameter::Listener
{
public:
	ParameterCcLearn(PluginBundle* bundle);
	~ParameterCcLearn();

	//==============================================================================
	void processCc(int instanceIndex, int ccNumber, int ccValue, float sampleOffset);
	void setCcLearn(int ccNumber, int parameterIndex, float min, float max);
	void reset(bool notify = false);
	void startLearning();

	//==============================================================================
	void parameterValueChanged(int parameterIndex, float newValue) override;
	void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}

	//==============================================================================
	void getStateInformation(MemoryBlock& destData);
	void setStateInformation(const void* data, int sizeInBytes);

	//==============================================================================
	int getCcLearnedParameterIndex() { return paramIndex; }
	int getCcSource() { return ccSource; }
	bool isLearning() { return _isLearning; }
	bool hasLearned() { return _hasLearned; }

	//==============================================================================
	void showStatus();

private:
	//==============================================================================
	void resetTempValues();
	bool validLearn();
	void stopLearning();

	String updateProgressNote(String otherMsg = "");

	//==============================================================================
	PluginBundle* _bundle;
	const Array<AudioProcessorParameter*>* _parameters{ nullptr };
	std::atomic<int> paramIndex{ -1 }, ccSource{ -1 };
	std::atomic<float> learnedCcMin{ FP_INFINITE }, learnedCcMax{ -FP_INFINITE };
	std::atomic<bool> _isLearning = false, _hasLearned = false;

	std::atomic<int> tmpParamIndex{ -1 }, tmpCcSource{ -1 };
	std::atomic<float> tmpLearnedCcMin{ FP_INFINITE }, tmpLearnedCcMax{ -FP_INFINITE };

	//==============================================================================
	class ProgressWindow : public DocumentWindow
	{
	public:
		ProgressWindow(ParameterCcLearn& parent, Colour backgroundColor);
		~ProgressWindow();

		ParameterCcLearn& getParent() { return _parent; }

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

			//==============================================================================
			void paint(Graphics& g) override;
			void resized() override;

			//==============================================================================
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

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterCcLearn)
};
