#pragma once
#include <BWAPI.h>

namespace KasoBot {

	class ConfigModule
	{
	private:
		ConfigModule();
		~ConfigModule();
		static ConfigModule* _instance;

		int _maxWorkersTotal = 80;
		int _maxWorkersPerMineral = 3;
		int _maxWorkersPerGas = 3;
		int _saturatedMineral = 2;
		int _saturatedGas = 3;
		int _startGasAt = 10;
		int _startGasAtBase = 5;

		bool _debugMap = false;
		bool _debugWorkers = false;
		bool _debugArmy = false;
		bool _debugBuildOrder = false;
		bool _debugStrategy = false;

	public:
		static ConfigModule* Instance();

		void Init();

		int MaxWorkersTotal() { return _maxWorkersTotal; }
		int MaxWorkersPerMineral() { return _maxWorkersPerMineral; }
		int MaxWorkersPerGas() { return _maxWorkersPerGas; }
		int SaturatedMineral() { return _saturatedMineral; }
		int SaturatedGas() { return _saturatedGas; }
		int StartGasAt() { return _startGasAt; }

		bool DebugMap() { return _debugMap; }
		bool DebugWorkers() { return _debugWorkers; }
		bool DebugArmy() { return _debugArmy; }
		bool DebugBuildOrder() { return _debugBuildOrder; }
		bool DebugStrategy() { return _debugStrategy; }
	};

	namespace Config {
		namespace Workers {

			//max allowed worker number globally
			int MaxGlobal(); 

			//max workers allowed per one mineral
			int MaxPerMineral(); 
			
			//max workers allowed per one refinery
			int MaxPerGas();

			//ideal number of workers allowed per one mineral
			int SaturationPerMineral();

			//ideal number of workers per one refinery
			int SaturationPerGas();

			//number of workers mining minerals when gas mining should start in this expansion
			int StartGasAt();
		}

		namespace Debug {

			//@return whether to draw map debug info on screen
			bool Map();

			//@return whether to draw worker debug info on screen
			bool Workers();

			//@return whether to draw unit debug info on screen
			bool Army();

			//@return whether to draw build order debug info on screen
			bool BuildOrder();

			//@return whether to draw strategy debug info on screen
			bool Strategy();
		}
	}
}


