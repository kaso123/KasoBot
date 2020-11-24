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
		int _repairMineralBlock = 100;

		int _unitOrderDelay = 7;
		int _unitOrderDistSimilarity = 50;
		int _clearTileLock = 150;
		int _enemyPositionResetFrames = 50;
		int _enemyArmyRange = 6;
		int _armyRange = 4;
		int _hiddenPositionResetFrames = 2000;
		int _hiddenBaseResetFrames = 1000;
		int _scoutBaseRadius = 200;
		int _enemyThreatRadius = 500;
		int _holdPositionDistance = 60;

		float _freeSupplyMultiplier = 1.5f;
		int _buildTimeout = 1000;
		float _maxUnitProportion = 2.0f;

		bool _debugMap = false;
		bool _debugWorkers = false;
		bool _debugArmy = false;
		bool _debugTasks = false;
		bool _debugBuildOrder = false;
		bool _debugStrategy = false;
		bool _debugOrders = false;
		bool _debugBases = false;
		bool _debugResources = false;
		bool _debugEnemy = false;

		int _minArmySupply = 60;
		int _maxArmySupply = 120;
		int _minAirArmySupply = 6;
		int _armySupplyIncrease = 8;
		int _maxAttackTasks = 2;
		int _maxScoutTasks = 1;
		int _firstScoutSupply = 8;
		int _skipOpenerAt = 4320; //3 minutes
		int _bunkerWorkers = 3;
		int _maxTasksPerArea = 2;
		int _scoutTasksStart = 4320; //3 minutes
		int _scoutRushStart = 1400;
		int _scoutTimeout = 700;
		int _scanBaseEnergy = 150;

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
		int RepairMineralBlock() { return _repairMineralBlock; }

		int UnitOrderDelay() { return _unitOrderDelay; }
		int UnitOrderDistSimilarity() { return _unitOrderDistSimilarity; }
		int ClearTileLock() { return _clearTileLock; }
		int EnemyPositionResetFrames() { return _enemyPositionResetFrames; }
		int EnemyArmyRange() { return _enemyArmyRange; }
		int ArmyRange() { return _armyRange; }
		int HiddenPositionResetFrames() { return _hiddenPositionResetFrames; }
		int HiddenBaseResetFrames() { return _hiddenBaseResetFrames; }
		int ScoutBaseRadius() { return _scoutBaseRadius; }
		int EnemyThreatRadius() { return _enemyThreatRadius; }
		int HoldPositionDistance() { return _holdPositionDistance; }

		float FreeSupplyMultiplier() { return _freeSupplyMultiplier; }
		int BuildTimeout() { return _buildTimeout; }
		float MaxUnitProportion() { return _maxUnitProportion; }

		bool DebugMap() { return _debugMap; }
		bool DebugWorkers() { return _debugWorkers; }
		bool DebugArmy() { return _debugArmy; }
		bool DebugTasks() { return _debugTasks; }
		bool DebugBuildOrder() { return _debugBuildOrder; }
		bool DebugStrategy() { return _debugStrategy; }
		bool DebugOrders() { return _debugOrders; }
		bool DebugBases() { return _debugBases; }
		bool DebugResources() { return _debugResources; }
		bool DebugEnemy() { return _debugEnemy; }

		int MinArmySupply() { return _minArmySupply; }
		int MaxArmySupply() { return _maxArmySupply; }
		int MinAirArmySupply() { return _minAirArmySupply; }
		int ArmySupplyIncrease() { return _armySupplyIncrease; }
		int MaxAttackTasks() { return _maxAttackTasks; }
		int MaxScoutTasks() { return _maxScoutTasks; }
		int FirstScoutSupply() { return _firstScoutSupply; }
		int SkipOpenerAt() { return _skipOpenerAt; }
		int BunkerWorkers() { return _bunkerWorkers; }
		int MaxTasksPerArea() { return _maxTasksPerArea; }
		int ScoutTasksStart() { return _scoutTasksStart; }
		int ScoutRushStart() { return _scoutRushStart; }
		int ScoutTimeout() { return _scoutTimeout; }
		int ScanBaseEnergy() { return _scanBaseEnergy; }
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

			//minimum mineral amount when repairs can be done (except bunker)
			int RepairMineralBlock();
		}

		namespace Units {

			//number of frames between allowing to issue new orders
			int OrderDelay();

			//distance from new order to current order when it is ignored and not sent to BWAPI
			int OrderDistSimilarity();

			//number of frames units ingore commands when they are making space for buildings
			int ClearTileLock();

			//number of frames between each position saving for enemies
			int EnemyPositionResetFrames();

			//max allowed distance from center point of army to unit to be considered part of that army (in Tiles!)
			int EnemyArmyRange();

			//distance from center of army when units are moved closer to center (in Tiles!)
			int ArmyRange();

			//number of frames when last seen position of hidden units should be reset to unknown
			int HiddenPositionResetFrames();
			
			//number of frames when empty base should switch state to unknown
			int HiddenBaseResetFrames();

			//radius of circle when scouting around enemy base
			int ScoutBaseRadius();

			//distance from any building when enemy army is considered to be threat
			int EnemyThreatRadius();

			//how close should unit be to target to order HoldPosition instead of AttackMove
			int HoldPositionDistance();


		}

		namespace Production {

			//how much more available supply we want to have in comparison to used supply 
			float FreeSupplyMultiplier();

			//number of frames we should wait to restart building when worker or building died
			int BuildTimeout();

			//proportion that single unit type can go over its limit in strategy
			float MaxUnitProportion();
		}

		namespace Debug {

			//@return whether to draw map debug info on screen
			bool Map();

			//@return whether to draw worker debug info on screen
			bool Workers();

			//@return whether to draw unit debug info on screen
			bool Army();

			//@return whether to draw tasks for army on screen
			bool Tasks();

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

			//@return how much supply can be in one army to start attack task, will be increased
			int MinArmySupply();

			//@return maximum amount of supply one army can reach after increasing of minArmySupply
			int MaxArmySupply();

			//@return how much supply can be in one air army to start own tasks
			int MinAirArmySupply();

			//@return supply addition to min army size when attacking army dies
			int ArmySupplyIncrease();

			//@return maximum number of attack tasks active in one moment
			int MaxAttackTasks();

			//@return maximum number of scout tasks active in one moment
			int MaxScoutTasks();

			//@return number of workers when first scout should start
			int FirstScoutSupply();

			//@return number of frames when we no longer do openers of strategies after switch
			int SkipOpenerAt();

			//@return max number of workers that should be assigned to repair bunker when attacked
			int BunkerWorkers();

			//@return max number of attack tasks that can be created for single area
			int MaxTasksPerArea();

			//@return frame when scout tasks are starting to be created
			int ScoutTasksStart();

			//@return frame when scouting for cannon rush should start
			int ScoutRushStart();

			//@return how many frames to wait between scouts when scout dies
			int ScoutTimeout();

			//@return minimal saved energy on comsat stations to scan enemy bases
			int ScanBaseEnergy();
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

			//@return whether this type has all requirements satisfied at this moment
			bool CanBuild(BWAPI::UnitType type);
		}
	}
}


