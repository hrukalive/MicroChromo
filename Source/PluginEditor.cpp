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
MicroChromoAudioProcessorEditor::MicroChromoAudioProcessorEditor (MicroChromoAudioProcessor& p)
    : AudioProcessorEditor (&p), 
	processor (p), 
	appProperties(p.getApplicationProperties()),
	knownPluginList(p.getKnownPluginList()),
	formatManager(p.getAudioPluginFormatManager()),
	mainProcessor(p.getAudioProcessorGraph())
{
	menuBar.reset(new MenuBarComponent(this));
	menuBar->setVisible(true);

	synthBtn.reset(new TextButton("Synth"));
	psBtn.reset(new TextButton("PitchShift"));
	synthLabel.reset(new Label("SynthLabel", "<empty>"));
	psLabel.reset(new Label("PitchShiftLabel", "<empty>"));
	synthBtn->addMouseListener(this, true);
	psBtn->addMouseListener(this, true);

	commandManager.setFirstCommandTarget(this);
	setApplicationCommandManagerToWatch(&commandManager);
	commandManager.registerAllCommandsForTarget(this);
	addKeyListener(commandManager.getKeyMappings());

	pluginSortMethod = (KnownPluginList::SortMethod)(appProperties.getUserSettings()->getIntValue("pluginSortMethod", KnownPluginList::sortByManufacturer));
	knownPluginList.addChangeListener(this);

	lnf.setColour(foleys::LevelMeter::lmMeterGradientLowColour, juce::Colours::green);
	meter.setLookAndFeel(&lnf);
	meter.setMeterSource(&processor.getMeterSource());

	addAndMakeVisible(menuBar.get());
	addAndMakeVisible(synthBtn.get());
	addAndMakeVisible(psBtn.get());
	addAndMakeVisible(synthLabel.get());
	addAndMakeVisible(psLabel.get());
	addAndMakeVisible(meter);

	setSize(400, 300);
}

MicroChromoAudioProcessorEditor::~MicroChromoAudioProcessorEditor()
{
	knownPluginList.removeChangeListener(this);
	synthBtn->removeMouseListener(this);
	psBtn->removeMouseListener(this);
#if JUCE_MAC
	MenuBarModel::setMacMainMenu(nullptr);
#endif
	menuBar = nullptr;
	synthBtn = nullptr;
	psBtn = nullptr;
	synthLabel = nullptr;
	psLabel = nullptr;
	meter.setLookAndFeel(nullptr);
}

//==============================================================================
void MicroChromoAudioProcessorEditor::mouseDown(const MouseEvent& e)
{
	auto common = [this](int r, int slot) {
		auto pluginId = slot == 0 ? this->synthId : this->psId;
		switch (r)
		{
		case 1:  showWindow(PluginWindow::Type::normal, pluginId); break;
		case 2:  showWindow(PluginWindow::Type::programs, pluginId); break;
		case 3:  showWindow(PluginWindow::Type::generic, pluginId); break;
		case 4:  showWindow(PluginWindow::Type::debug, pluginId); break;
		default:
		{
			auto types = knownPluginList.getTypes();
			int result = KnownPluginList::getIndexChosenByMenu(types, r);
			auto& desc = types.getReference(result);
			processor.addPlugin(
				desc,
				slot, 0,
				[&]()
			{
				this->graphUpdated();
			});
		}
		}
	};
	if (e.eventComponent == synthBtn.get())
	{
		showPopupMenu(0, e.position.toInt(), [this, common](int r) { common(r, 0); });
	}
	else if (e.eventComponent == psBtn.get())
	{
		showPopupMenu(0, e.position.toInt(), [this, common](int r) { common(r, 1); });
	}
}

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
	b.removeFromLeft(10);
	b.removeFromTop(10);

	auto tmp = b.removeFromTop(40);
	synthBtn->setBounds(tmp.removeFromLeft(100));
	tmp.removeFromLeft(10);
	synthLabel->setBounds(tmp.removeFromLeft(100));

	b.removeFromTop(10);

	tmp = b.removeFromTop(40);
	psBtn->setBounds(tmp.removeFromLeft(100));
	tmp.removeFromLeft(10);
	psLabel->setBounds(tmp.removeFromLeft(100));

	b.removeFromTop(10);
	meter.setBounds(b.removeFromLeft(60).withTrimmedBottom(10));
}

void MicroChromoAudioProcessorEditor::buttonClicked(Button* btn)
{
}

void MicroChromoAudioProcessorEditor::showPopupMenu(int type, Point<int> position, std::function<void(int)> callback)
{
	floatMenu.reset(new PopupMenu);

	floatMenu->addItem(1, "Show plugin GUI");
	floatMenu->addItem(2, "Show all programs");
	floatMenu->addItem(3, "Show all parameters");
	floatMenu->addItem(4, "Show debug log");
	floatMenu->addSeparator();
	KnownPluginList::addToMenu(*floatMenu, knownPluginList.getTypes(), pluginSortMethod);
	floatMenu->showMenuAsync({}, ModalCallbackFunction::create([this, callback](int r)
		{
			if (r > 0)
				callback(r);
		}));
}

void MicroChromoAudioProcessorEditor::graphUpdated()
{ 
	for (auto* node : mainProcessor.getNodes())
	{
		if (static_cast<int>(node->properties["copy_number"]) == 0)
		{
			if (static_cast<int>(node->properties["slot_number"]) == 0)
			{
				synthLabel->setText(node->getProcessor()->getName(), NotificationType::dontSendNotification);
				synthId = node->nodeID;
			}
			else if (static_cast<int>(node->properties["slot_number"]) == 1)
			{
				psLabel->setText(node->getProcessor()->getName(), NotificationType::dontSendNotification);
				psId = node->nodeID;
			}
		}
	}
}

void MicroChromoAudioProcessorEditor::showWindow(PluginWindow::Type type, AudioProcessorGraph::NodeID pluginID)
{
	if (auto node = mainProcessor.getNodeForId(pluginID))
	{
		for (auto* w : activePluginWindows)
		{
			if (w->node.get() == node && w->type == type)
			{
				w->toFront(true);
				return;
			}
		}
		if (auto* processor = node->getProcessor())
		{
			activePluginWindows.add(new PluginWindow(node, type, activePluginWindows))->toFront(true);
		}
	}
}
