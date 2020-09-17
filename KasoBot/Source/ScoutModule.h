#pragma once
#include <BWAPI.h>

namespace KasoBot {

	struct EnemyUnit {
		int id;
		BWAPI::TilePosition lastPos;
		BWAPI::UnitType type;
		bool hidden;
		
		EnemyUnit(BWAPI::Unit unit) : 
			id(unit->getID()), lastPos(unit->getTilePosition())
			,type(unit->getType()), hidden(false) 
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

		//cycle all enemies and save new position for visible enemies
		void ResetEnemyInfo();

	public:
		static ScoutModule* Instance();

		void OnFrame();

		//save info about enemy unit or update it when already seen before
		void EnemyDiscovered(BWAPI::Unit unit);

		//set this unit info to hidden
		void EnemyHidden(BWAPI::Unit unit);

		//remove enemy unit info from list
		void EnemyDestroyed(BWAPI::Unit unit);

		//getters and setters

		const std::unordered_map<BWAPI::UnitType, EnemyList, std::hash<int>>& GetEnemies() const { return _enemies; }

	};
}


