#include "stdafx.h"
#include "GameGUI.h"

#include <Core.h>
#include <LibRocketGUIManager.h>
#include <util/ConfigSettings.h>
#include "OnClickListener.h"
#include "OnClickEndGameListener.h"

void setupGUIDocuments(Game *game)
{
	LibRocketGUIManager *manager = Core::getInGameLibRocketGUIManager();
	std::string mediaPath;
	ConfigSettings::config->getValue("MediaPath", mediaPath);

	std::string guiPath = mediaPath + "DefaultAssets\\LibRocketGUI\\InGame\\";
	int i = manager->addWindow(guiPath + "window_rocket.rml", true);
	manager->getWindow(i)->Hide();
	i = manager->addWindow(guiPath + "window_start_screen.rml", true);
	manager->getWindow(i)->Hide();
	i = manager->addWindow(guiPath + "window_score.rml", true);
	manager->getWindow(i)->Hide();

	Rocket::Core::ElementDocument* marty = manager->getWindow(1);
	Rocket::Core::EventListenerInstancer;
	Rocket::Core::EventListener * startGameListener = new OnClickListener(game);
	Rocket::Core::EventListener * endGameListener = new OnClickEndGameListener(game);
	marty->GetFirstChild()->GetElementById("startgame")->AddEventListener("click", startGameListener);
	marty->GetFirstChild()->GetElementById("exitgame")->AddEventListener("click", endGameListener);
}