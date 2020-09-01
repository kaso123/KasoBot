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
		int _startGasAfter = 10;
		int _buildStartDistance = 50;

		int _unitOrderDelay = 7;

		bool _debugMap = false;
		bool _debugWorkers = false;
		bool _debugArmy = false;
		bool _debugBuildOrder = false;
		bool _debugStrategy = false;
		bool _debugOrders = false;

	public:
		static ConfigModule* Instance();

		void Init();

		int MaxWorkersTotal() { return _maxWorkersTotal; }
		int MaxWorkersPerMineral() { return _maxWorkersPerMineral; }
		int MaxWorkersPerGas() { return _maxWorkersPerGas; }
		int SaturatedMineral() { return _saturatedMineral; }
		int SaturatedGas() { return _saturatedGas; }
		int StartGasAfter() { return _startGasAfter; }
		int BuildStartDistance() { return _buildStartDistance; }

		int UnitOrderDelay() { return _unitOrderDelay; }

		bool DebugMap() { return _debugMap; }
		bool DebugWorkers() { return _debugWorkers; }
		bool DebugArmy() { return _debugArmy; }
		bool DebugBuildOrder() { return _debugBuildOrder; }
		bool DebugStrategy() { return _debugStrategy; }
		bool DebugOrders() { return _debugOrders; }
		
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
			int StartGasAfter();

			//distance from build position when worker receives build command
			int BuildStartDistance();
		}

		namespace Units {

			//number of frames between allowing to issue new orders
			int OrderDelay();
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

			//@return whether to draw BWAPI orders for units on screen
			bool Orders();
		}
	}
}


