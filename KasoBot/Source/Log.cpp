#include "Log.h"

#include <iostream>
#include <chrono>
#include <ctime>

using namespace KasoBot;

Log* Log::_instance = 0;

Log::Log()
{
	for (auto p : BWAPI::Broodwar->getPlayers())
	{
		if (BWAPI::Broodwar->self()->isEnemy(p))
		{

			std::time_t now_c = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			std::tm * ptm = std::localtime(&now_c);
			char buffer[32];
			std::strftime(buffer, 32, "%Y-%m-%d_%H-%M", ptm);

			_name = "bwapi-data/write/" + p->getName() + buffer + ".txt";

			BWAPI::Broodwar->sendText("%s", _name.c_str());
			std::ofstream log(_name.c_str(), std::ofstream::app);
			log << "Game started" << std::endl;
			log.close();

			break;
		}
	}
}

Log::~Log()
{
	delete(_instance);
}

Log* Log::Instance()
{
	if (!_instance)
		_instance = new Log;
	return _instance;
}

void Log::Assert(bool value, const char* message)
{
	if (value)
		return;

	_logFile.open(_name.c_str(), std::ofstream::app);
	std::ofstream assertLog("bwapi-data/write/assert.txt", std::ofstream::app);
	
	if (!_logFile)
	{
		BWAPI::Broodwar->sendText(message);
		return;
	}

	_logFile << message << " frame:" << BWAPI::Broodwar->getFrameCount() << std::endl;
	assertLog << message << " frame:" << BWAPI::Broodwar->getFrameCount() << std::endl;
		
	_logFile.close();
	assertLog.close();
}

void Log::Strategy(const char* strat, const char* enemyStrat)
{
	_logFile.open(_name.c_str(), std::ofstream::app);

	if (!_logFile)
	{
		BWAPI::Broodwar->sendText("Strategy switch: %s  Because enemy is doing: %s \n", strat, enemyStrat);
		return;
	}

	_logFile << "Strategy switch: " << strat << "  Because enemy is doing: " << enemyStrat << std::endl;

	_logFile.close();
}
