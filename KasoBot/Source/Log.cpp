#include "Log.h"
#include <fstream>

using namespace KasoBot;

namespace {
	std::string _fileName = "bwapi-data/write/default.txt";
}


void Log::CreateFileName()
{
	for (auto p : BWAPI::Broodwar->getPlayers())
	{
		if (BWAPI::Broodwar->self()->isEnemy(p))
		{
			_fileName = "bwapi-data/write/default.txt";
			BWAPI::Broodwar->sendText("%s",_fileName.c_str());
			std::ofstream log(_fileName.c_str(), std::ofstream::app);
			log << "Game started\n";
			log.close();

			return;
		}
	}

}

void Log::Assert(bool value, const char* message)
{
	if (value)
		return;

	std::ofstream log("bwapi-data/write/assert.txt", std::ofstream::app);
	
	if (!log)
	{
		BWAPI::Broodwar->sendText(message);
		return;
	}

	log << message << " frame:" << BWAPI::Broodwar->getFrameCount() << "\n";
		
	log.close();
}

void Log::Strategy(const char* strat, const char* enemyStrat)
{
	std::ofstream log("bwapi-data/write/strat.txt", std::ofstream::app);

	if (!log)
	{
		BWAPI::Broodwar->sendText("Strategy switch: %s  Because enemy is doing: %s \n", strat, enemyStrat);
		return;
	}

	log << "Strategy switch: " << strat << "  Because enemy is doing: " << enemyStrat << "\n";

	log.close();
}
