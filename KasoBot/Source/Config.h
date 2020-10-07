#pragma once
#include <BWAPI.h>

namespace KasoBot {

	namespace Production {
		struct TechMacro;
	}

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
		int _workerResourceValue = 5;

		int _unitOrderDelay = 7;
		int _clearTileLock = 150;
		int _enemyPositionResetFrames = 50;
		int _hiddenPositionResetFrames = 2000;
		int _hiddenBaseResetFrames = 1000;
		int _scoutBaseRadius = 200;

		float _freeSupplyMultiplier = 1.5f;

		bool _debugMap = false;
		bool _debugWorkers = false;
		bool _debugArmy = false;
		bool _debugBuildOrder = false;
		bool _debugStrategy = false;
		bool _debugOrders = false;
		bool _debugBases = false;
		bool _debugResources = false;
		bool _debugEnemy = false;

		int _maxArmySupply = 60;
		int _firstScoutSupply = 8;
		int _skipOpenerAt = 4320; //3 minutes

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
		int WorkerResourceValue() { return _workerResourceValue; }

		int UnitOrderDelay() { return _unitOrderDelay; }
		int ClearTileLock() { return _clearTileLock; }
		int EnemyPositionResetFrames() { return _enemyPositionResetFrames; }
		int HiddenPositionResetFrames() { return _hiddenPositionResetFrames; }
		int HiddenBaseResetFrames() { return _hiddenBaseResetFrames; }
		int ScoutBaseRadius() { return _scoutBaseRadius; }

		float FreeSupplyMultiplier() { return _freeSupplyMultiplier; }

		bool DebugMap() { return _debugMap; }
		bool DebugWorkers() { return _debugWorkers; }
		bool DebugArmy() { return _debugArmy; }
		bool DebugBuildOrder() { return _debugBuildOrder; }
		bool DebugStrategy() { return _debugStrategy; }
		bool DebugOrders() { return _debugOrders; }
		bool DebugBases() { return _debugBases; }
		bool DebugResources() { return _debugResources; }
		bool DebugEnemy() { return _debugEnemy; }

		int MaxArmySupply() { return _maxArmySupply; }
		int FirstScoutSupply() { return _firstScoutSupply; }
		int SkipOpenerAt() { return _skipOpenerAt; }
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

			//number of minerals/gas we expect one worker to mine until worker gets to build location when constructing
			int WorkerResourceValue();
		}

		namespace Units {

			//number of frames between allowing to issue new orders
			int OrderDelay();

			//number of frames units ingore commands when they are making space for buildings
			int ClearTileLock();

			//number of frames between each position saving for enemies
			int EnemyPositionResetFrames();

			//number of frames when last seen position of hidden units should be reset to unknown
			int HiddenPositionResetFrames();
			
			//number of frames when empty base should switch state to unknown
			int HiddenBaseResetFrames();

			//radius of circle when scouting around enemy base
			int ScoutBaseRadius();
		}

		namespace Production {

			//hom much more available supply we want to have in comparison to used supply 
			float FreeSupplyMultiplier();
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

			//@return whether to draw info about bases on screen
			bool Bases();

			//@return whether to draw info about reserved, mined and lost resources
			bool Resources();

			//@return whether to draw info about enemy
			bool Enemy();
		}

		namespace Strategy {

			//@return how much supply can be in one army
			int MaxArmySupply();

			//@return number of workers when first scout should start
			int FirstScoutSupply();

			//@return number of frames when we no longer do openers of strategies after switch
			int SkipOpenerAt();
		}

		namespace Utils {
			//@return BWAPI type parsed from config string
			BWAPI::UnitType TypeFromString(std::string input);

			//@return Macro item (unit, tech or upgrade) parsed from config string
			KasoBot::Production::TechMacro TechTypeFromString(std::string input);

			//@return next type that needs to be built to unlock inputed type, UnitType::None if already unlocked
			BWAPI::UnitType NextPrerequisite(BWAPI::UnitType type);

			//@return next type that needs to be built to unlock inputed type, UnitType::None if already unlocked
			BWAPI::UnitType NextPrerequisite(BWAPI::TechType type);

			//@return next type that needs to be built to unlock inputed type, UnitType::None if already unlocked
			BWAPI::UnitType NextPrerequisite(BWAPI::UpgradeType type);
		}
	}
}


