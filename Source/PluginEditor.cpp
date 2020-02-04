/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
class MicroChromoAudioProcessorEditor::PluginListWindow : public DocumentWindow
{
public:
	PluginListWindow(MicroChromoAudioProcessorEditor &mw, AudioPluginFormatManager& pluginFormatManager)
		: DocumentWindow("Available Plugins",
			LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId),
			DocumentWindow::minimiseButton | DocumentWindow::closeButton),
		owner(mw)
	{
		auto deadMansPedalFile = owner.appProperties->getUserSettings()->getFile().getSiblingFile("RecentlyCrashedPluginsList");

		setContentOwned(new PluginListComponent(pluginFormatManager, owner.knownPluginList, deadMansPedalFile, owner.appProperties->getUserSettings(), true), true);

		setResizable(true, false);
		setResizeLimits(300, 400, 800, 1500);
		setTopLeftPosition(60, 60);

		setVisible(true);
	}

	~PluginListWindow()
	{
		clearContentComponent();
	}

	void closeButtonPressed()
	{
		owner.pluginListWindow = nullptr;
	}

private:
	MicroChromoAudioProcessorEditor& owner;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginListWindow)
};

MicroChromoAudioProcessorEditor::MicroChromoAudioProcessorEditor (MicroChromoAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
	PropertiesFile::Options options;
	options.folderName = "MicroChromo";
	options.applicationName = "MicroChromo Host";
	options.filenameSuffix = "settings";
	options.osxLibrarySubFolder = "Preferences";

	appProperties.reset(new ApplicationProperties());
	appProperties->setStorageParameters(options);

	formatManager.addDefaultFormats();
    setSize (400, 300);

	button1.reset(new TextButton("test"));
	addAndMakeVisible(button1.get());
	button1->addListener(this);
	button1->setBounds(10, 10, 100, 50);

	if (auto savedPluginList = appProperties->getUserSettings()->getXmlValue("pluginList"))
		knownPluginList.recreateFromXml(*savedPluginList);
	pluginSortMethod = (KnownPluginList::SortMethod)(appProperties->getUserSettings()->getIntValue("pluginSortMethod", KnownPluginList::sortByManufacturer));
	knownPluginList.addChangeListener(this);

	//AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "Editor init", "INIT");
}

MicroChromoAudioProcessorEditor::~MicroChromoAudioProcessorEditor()
{
	knownPluginList.removeChangeListener(this);

	appProperties = nullptr;
	pluginListWindow = nullptr;
	button1 = nullptr;
}

//==============================================================================
void MicroChromoAudioProcessorEditor::changeListenerCallback(ChangeBroadcaster* changed)
{
	if (changed == &knownPluginList)
	{
		if (auto savedPluginList = std::unique_ptr<XmlElement>(knownPluginList.createXml()))
		{
			appProperties->getUserSettings()->setValue("pluginList", savedPluginList.get());
			appProperties->saveIfNeeded();
		}
	}
}

//==============================================================================
void MicroChromoAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setColour (Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), Justification::centred, 1);
}

void MicroChromoAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

void MicroChromoAudioProcessorEditor::buttonClicked(Button* btn)
{
	if (btn == button1.get())
	{
		if (pluginListWindow == nullptr)
			pluginListWindow.reset(new PluginListWindow(*this, formatManager));
		pluginListWindow->toFront(true);
	}
}
