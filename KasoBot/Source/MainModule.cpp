#include "MainModule.h"
#include "WorkersModule.h"
#include "ProductionModule.h"
#include "MapModule.h"
#include "ArmyModule.h"
#include "Config.h"
#include <iostream>

using namespace BWAPI;
using namespace KasoBot;

void MainModule::onStart()
{  
	BWAPI::Broodwar->sendText("Good Fun, have Luck!");

	// Enable the UserInput flag, which allows us to control the bot and type messages.
	Broodwar->enableFlag(Flag::UserInput);

	Map::Global::Initialize();
	ConfigModule::Instance()->Init();

	WorkersModule::Instance()->OnStart();
}

void MainModule::onEnd(bool isWinner)
{
}

void MainModule::onFrame()
{
  // Called once every game frame

  // Display the game frame rate as text in the upper left area of the screen
  Broodwar->drawTextScreen(200, 0,  "FPS: %d", Broodwar->getFPS() );

  WorkersModule::Instance()->OnFrame();
}

void MainModule::onSendText(std::string text)
{
  // Send the text to the game if it is not being processed.
  Broodwar->sendText("%s", text.c_str());

  // Make sure to use %s and pass the text as a parameter,
  // otherwise you may run into problems when you use the %(percent) character!
}

void MainModule::onReceiveText(BWAPI::Player player, std::string text)
{
  // Parse the received text
  Broodwar << player->getName() << " said \"" << text << "\"" << std::endl;
}

void MainModule::onPlayerLeft(BWAPI::Player player)
{
  // Interact verbally with the other players in the game by
  // announcing that the other player has left.
  Broodwar->sendText("LOL, Get Rekt %s!", player->getName().c_str());
}

void MainModule::onNukeDetect(BWAPI::Position target)
{
}

void MainModule::onUnitDiscover(BWAPI::Unit unit)
{
}

void MainModule::onUnitEvade(BWAPI::Unit unit)
{
}

void MainModule::onUnitShow(BWAPI::Unit unit)
{
}

void MainModule::onUnitHide(BWAPI::Unit unit)
{
}

void MainModule::onUnitCreate(BWAPI::Unit unit)
{
}

void MainModule::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit->getPlayer() == Broodwar->self())
	{
		//remove unit from lists
		if (unit->getType().isBuilding())
		{
			if (unit->getType().isResourceDepot())
			{
				WorkersModule::Instance()->ExpansionDestroyed(unit);
			}
			else
			{
				ProductionModule::Instance()->RemoveBuilding(unit);
			}
		}
		else
		{
			if (unit->getType().isWorker())
			{
				WorkersModule::Instance()->RemoveWorker(unit);
			}
			else
			{
				ProductionModule::Instance()->RemoveUnit(unit);
			}
		}
	}
}

void MainModule::onUnitMorph(BWAPI::Unit unit)
{
}

void MainModule::onUnitRenegade(BWAPI::Unit unit)
{
}

void MainModule::onSaveGame(std::string gameName)
{
}

void MainModule::onUnitComplete(BWAPI::Unit unit)
{
	if (unit->getPlayer() == Broodwar->self())
	{
		//add new units to lists
		if (unit->getType().isBuilding())
		{			
			if (unit->getType().isResourceDepot())
			{
				WorkersModule::Instance()->ExpansionCreated(unit);
			}
			else if (unit->getType().isRefinery())
			{
				WorkersModule::Instance()->RefineryCreated(unit);
			}
			else
			{
				ProductionModule::Instance()->AddBuilding(unit);
			}
		}
		else
		{
			if (unit->getType().isWorker())
			{
				WorkersModule::Instance()->NewWorker(unit);
			}
			else
			{
				ProductionModule::Instance()->AddUnit(unit);
			}
		}
	}	
}
