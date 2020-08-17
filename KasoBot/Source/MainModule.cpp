#include "MainModule.h"
#include <iostream>

using namespace BWAPI;
using namespace Filter;

void MainModule::onStart()
{  
	Broodwar->sendText("Good Fun, have Luck!");

	// Enable the UserInput flag, which allows us to control the bot and type messages.
	Broodwar->enableFlag(Flag::UserInput);
}

void MainModule::onEnd(bool isWinner)
{
}

void MainModule::onFrame()
{
  // Called once every game frame

  // Display the game frame rate as text in the upper left area of the screen
  Broodwar->drawTextScreen(200, 0,  "FPS: %d", Broodwar->getFPS() );
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
}
