#pragma once
#include <BWAPI.h>

namespace KasoBot {
	namespace Log {

		//set log filename from enemy name and map name
		void CreateFileName();

		//write message to log if value = false
		void Assert(bool value, const char* message);

		void Strategy(const char* strat, const char* enemyStrat);
	}
}

