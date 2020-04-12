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
	void addCcLearn(int ccNumber, int parameterIndex, float min, float max);
	void removeCcLearn(int parameterIndex);
	void reset(bool notify = false);
	void startLearning();

	//==============================================================================
	void parameterValueChanged(int parameterIndex, float newValue) override;
	void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}

	//==============================================================================
	std::unique_ptr<XmlElement> createXml();
	void loadFromXml(const XmlElement* xml);

	//==============================================================================
	auto& getCcLearnedParameterIndex() { return learnedIndices; }
	int getCcSource() { return ccSource; }
	bool isLearning() { return _isLearning; }
	bool hasLearned() { return _hasLearned; }

	//==============================================================================
	void showStatus();

private:
	//==============================================================================
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
		float learnedCcMin{ FP_INFINITE }, learnedCcMax{ -FP_INFINITE };
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


	class StatusComponent : public Component, public TableListBoxModel
	{
	public:
		StatusComponent(ParameterCcLearn& parent);
		~StatusComponent() = default;

		//==============================================================================
		void paint(Graphics& g) override;
		void resized() override;
		void selectedRowsChanged(int row) override;

		int getNumRows() override;

		void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
		void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;

		int getColumnAutoSizeWidth(int columnId) override;
		void updateCc(int ccNumber);

	private:
		ParameterCcLearn& _parent;
		Array<CcItem>& items;

		int lastRow = -1;

		TextEditor ccNumberTextBox;
		TableListBox table{ {}, this };
		TextButton removeBtn{ "Remove" }, setBtn{ "Set" };
		Font font{ 14.0f };

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatusComponent)
	};

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterCcLearn)
};
