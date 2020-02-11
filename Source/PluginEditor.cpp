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
		auto deadMansPedalFile = owner.appProperties.getUserSettings()->getFile().getSiblingFile("RecentlyCrashedPluginsList");

		setContentOwned(new PluginListComponent(pluginFormatManager, owner.knownPluginList, deadMansPedalFile, owner.appProperties.getUserSettings(), true), true);

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

//==============================================================================
MicroChromoAudioProcessorEditor::MicroChromoAudioProcessorEditor (MicroChromoAudioProcessor& p, ApplicationProperties& _appProperties, KnownPluginList& _knownPluginList, AudioPluginFormatManager& _formatManager)
    : AudioProcessorEditor (&p), processor (p), appProperties(_appProperties), knownPluginList(_knownPluginList), formatManager(_formatManager)
{
	button1.reset(new TextButton("test"));
	button1->addListener(this);

	menuBar.reset(new MenuBarComponent(this));
	menuBar->setVisible(true);

	commandManager.setFirstCommandTarget(this);
	setApplicationCommandManagerToWatch(&commandManager);
	commandManager.registerAllCommandsForTarget(this);
	addKeyListener(commandManager.getKeyMappings());

	pluginSortMethod = (KnownPluginList::SortMethod)(appProperties.getUserSettings()->getIntValue("pluginSortMethod", KnownPluginList::sortByManufacturer));
	knownPluginList.addChangeListener(this);

	addAndMakeVisible(button1.get());
	addAndMakeVisible(menuBar.get());

	setSize(400, 300);
}

MicroChromoAudioProcessorEditor::~MicroChromoAudioProcessorEditor()
{
	knownPluginList.removeChangeListener(this);
#if JUCE_MAC
	MenuBarModel::setMacMainMenu(nullptr);
#endif
	button1 = nullptr;
	menuBar = nullptr;
}

//==============================================================================
void MicroChromoAudioProcessorEditor::changeListenerCallback(ChangeBroadcaster* changed)
{
	if (changed == &knownPluginList)
	{
		if (auto savedPluginList = std::unique_ptr<XmlElement>(knownPluginList.createXml()))
		{
			appProperties.getUserSettings()->setValue("pluginList", savedPluginList.get());
			appProperties.saveIfNeeded();
		}
	}
}

//==============================================================================
StringArray MicroChromoAudioProcessorEditor::getMenuBarNames()
{
	return { "File", "Plugins" };
}

PopupMenu MicroChromoAudioProcessorEditor::getMenuForIndex(int menuIndex, const String& /*menuName*/)
{
	PopupMenu menu;

	if (menuIndex == 0)
	{
		menu.addCommandItem(&commandManager, CommandIDs::openPluginScanner);
		menu.addCommandItem(&commandManager, CommandIDs::testCommand);
	}
	else if (menuIndex == 1)
	{
		KnownPluginList::addToMenu(menu, knownPluginList.getTypes(), pluginSortMethod);
	}

	return menu;
}

void MicroChromoAudioProcessorEditor::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
	if (topLevelMenuIndex == 1)
	{
		auto& types = knownPluginList.getTypes();
		int result = knownPluginList.getIndexChosenByMenu(menuItemID);
		auto& desc = types.getReference(result);

		Point<int> position{ proportionOfWidth(0.3f + Random::getSystemRandom().nextFloat() * 0.6f),
													proportionOfHeight(0.3f + Random::getSystemRandom().nextFloat() * 0.6f) };
		processor.addPlugin(desc, position.toDouble() / Point<double>((double)getWidth(), (double)getHeight()), [&]() { commandManager.invokeDirectly(CommandIDs::testCommand, true); });
	}
}

//==============================================================================
ApplicationCommandTarget* MicroChromoAudioProcessorEditor::getNextCommandTarget()
{
	return findFirstTargetParentComponent();
}

void MicroChromoAudioProcessorEditor::getAllCommands(Array<CommandID>& c)
{
	Array<CommandID> commands{
		CommandIDs::openPluginScanner,
		CommandIDs::testCommand
	};
	c.addArray(commands);
}

void MicroChromoAudioProcessorEditor::getCommandInfo(CommandID commandID, ApplicationCommandInfo& result)
{
	switch (commandID)
	{
	case CommandIDs::openPluginScanner:
		result.setInfo("Open Scanner", "Open the scanner for plugins", "File", 0);
#if JUCE_MAC
		result.addDefaultKeypress('q', ModifierKeys::commandModifier);
#else
		result.addDefaultKeypress('q', ModifierKeys::ctrlModifier);
#endif
		break;
	case CommandIDs::testCommand:
		result.setInfo("Test", "Test", "File", 0);
		break;
	default:
		break;
	}
}
bool MicroChromoAudioProcessorEditor::perform(const InvocationInfo& info)
{
	switch (info.commandID)
	{
	case CommandIDs::openPluginScanner:
	{
		if (pluginListWindow == nullptr)
			pluginListWindow.reset(new PluginListWindow(*this, formatManager));
		pluginListWindow->toFront(true);
		break;
	}
	case CommandIDs::testCommand:
		AlertWindow::showMessageBoxAsync(AlertWindow::InfoIcon, "Command Test", "TEST");
		break;
	default:
		return false;
	}
	return true;
}

//==============================================================================
void MicroChromoAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void MicroChromoAudioProcessorEditor::resized()
{
	auto b = getLocalBounds();
#if !JUCE_MAC
	menuBar->setBounds(b.removeFromTop(LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight()));
#endif
}

void MicroChromoAudioProcessorEditor::buttonClicked(Button* btn)
{
	if (btn == button1.get())
	{
		
	}
}
