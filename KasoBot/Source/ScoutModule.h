#pragma once
#include <BWAPI.h>

namespace BWEM {
	class Area;
	class Base;
}

namespace KasoBot {

	struct BaseInfo;
	class EnemyArmy;

	struct EnemyUnit {
		int _id;
		BWAPI::TilePosition _lastPos;
		int _lastSeenFrame;
		BWAPI::UnitType _type;
		bool _hidden;
		EnemyArmy* _army;
		
		EnemyUnit(BWAPI::Unit unit) : 
			_id(unit->getID()), _lastPos(unit->getTilePosition()), _army(nullptr)
			,_type(unit->getType()), _hidden(false), _lastSeenFrame(BWAPI::Broodwar->getFrameCount())
		{}
		~EnemyUnit();
	};

	typedef std::vector<std::unique_ptr<EnemyUnit>> EnemyList;

	class ScoutModule
	{
	private:
		ScoutModule();
		~ScoutModule();
		static ScoutModule* _instance;

		std::unordered_map<BWAPI::UnitType, EnemyList, std::hash<int>> _enemies; //List of enemy units we discovered
		std::vector<std::unique_ptr<BaseInfo>> _baseInfo; //list of baseInfo structs, should be accessed through bWEM, this is only for memory management

		std::vector<std::unique_ptr<EnemyArmy>> _armies; //list of enemy armies that are visible
		BWAPI::Race _enemyRace;

		int _scanTimeout; //frame when next scan can be done
		int _scanTimeoutTech; //frame when next techScan can be done

		const BWEM::Area* _enemyStart; //enemy starting area
		const BWEM::Area* _enemyNatural;

		//cycle all enemies and save new position for visible enemies
		void ResetEnemyInfo();

		//cycle all bases and check if they are visible and set appropriate status
		void ResetBaseInfo();

		//remove specific unit from specific type list
		void RemoveByID(int unitID, BWAPI::UnitType oldType);

		//check if this unit was another type before and remove it from list
		void CheckEnemyEvolution(BWAPI::Unit unit);

		//merge together armies that are close together
		void MergeArmies();

		//try to create defend task for every enemy army that is close to our base
		void CreateDefendTasks();

		//check all scannable enemies and scan if needed
		void ScanEnemies();

		//if scout worker died, use scan to periodicaly scout enemy base
		void ScanTech();

		//@return true if this type is defensive structure (these buildings are added to armies)
		bool IsDefenceBuilding(BWAPI::UnitType type);

	public:
		static ScoutModule* Instance();

		void OnFrame();
		void OnStart();

		//save info about enemy unit or update it when already seen before
		void EnemyDiscovered(BWAPI::Unit unit);

		//set this unit info to hidden
		void EnemyHidden(BWAPI::Unit unit);

		//remove enemy unit info from list
		void EnemyDestroyed(BWAPI::Unit unit);

		//@return true if first worker scout should be in progress 
		bool ShouldWorkerScout();

		//@return true if worker should be scouting our base for cannon rush 
		bool ShouldWorkerScoutRush();

		//@return number of enemy units/buildings of this type
		int GetCountOf(BWAPI::UnitType type);

		//return enemy race and if it is random set race according to seen units
		BWAPI::Race GetEnemyRace();

		//assign enemy unit to an existing army if close, or to new army
		void AssignToArmy(EnemyUnit* enemy);

		//@return true if one of enemy armies is worker rushing
		bool EnemyWorkerRush();

		//@return true if one of enemy armies looks like cannon rush
		bool EnemyCannonRush();

		//scan base if enough energy spared
		void ScanBase(const BWEM::Base& base);
		
		//getters and setters

		const std::unordered_map<BWAPI::UnitType, EnemyList, std::hash<int>>& GetEnemies() const { return _enemies; }
		const std::vector<std::unique_ptr<EnemyArmy>>& GetArmies() const { return _armies; }
		const BWEM::Area* EnemyStart() const { return _enemyStart; }
		const BWEM::Area* EnemyNatural() const { return _enemyNatural; }

	};
}


