#pragma once
#include <BWAPI.h>

namespace BWEM {
	class Area;
}

namespace KasoBot {

	struct BaseInfo;

	struct EnemyUnit {
		int id;
		BWAPI::TilePosition lastPos;
		int lastSeenFrame;
		BWAPI::UnitType type;
		bool hidden;
		
		EnemyUnit(BWAPI::Unit unit) : 
			id(unit->getID()), lastPos(unit->getTilePosition())
			,type(unit->getType()), hidden(false), lastSeenFrame(BWAPI::Broodwar->getFrameCount())
		{}
	};

	typedef std::vector<EnemyUnit> EnemyList;

	class ScoutModule
	{
	private:
		ScoutModule();
		~ScoutModule();
		static ScoutModule* _instance;

		std::unordered_map<BWAPI::UnitType, EnemyList, std::hash<int>> _enemies; //List of enemy units we discovered
		std::vector<std::unique_ptr<BaseInfo>> _baseInfo; //list of baseInfo structs, should be accessed through bWEM, this is only for memory management

		BWAPI::Race _enemyRace;

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

		//@return number of enemy units/buildings of this type
		int GetCountOf(BWAPI::UnitType type);

		//return enemy race and if it is random set race according to seen units
		BWAPI::Race GetEnemyRace();



		//getters and setters

		const std::unordered_map<BWAPI::UnitType, EnemyList, std::hash<int>>& GetEnemies() const { return _enemies; }
		const BWEM::Area* EnemyStart() const { return _enemyStart; }
		const BWEM::Area* EnemyNatural() const { return _enemyNatural; }

	};
}


