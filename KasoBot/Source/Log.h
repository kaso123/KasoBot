#pragma once
#include <BWAPI.h>
#include <fstream>

namespace KasoBot {
	class Log {
	private:
		Log();
		~Log();
		static Log* _instance;

		std::string _name;
		std::ofstream _logFile;
	public:

		static Log* Instance();

		//write message to log if value = false
		void Assert(bool value, const char* message);

		void Strategy(const char* strat, const char* enemyStrat);
	};
}

