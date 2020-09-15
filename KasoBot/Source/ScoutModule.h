#pragma once
#include <BWAPI.h>

namespace KasoBot {

	struct EnemyUnit {
		int id;
		BWAPI::TilePosition lastPos;
		BWAPI::UnitType type;
		
		EnemyUnit(BWAPI::Unit unit) : 
			id(unit->getID()), lastPos(unit->getTilePosition())
			,type(unit->getType()) 
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

	public:
		static ScoutModule* Instance();

		void EnemyDiscovered(BWAPI::Unit unit);

		void EnemyDestroyed(BWAPI::Unit unit);

		//getters and setters

		const std::unordered_map<BWAPI::UnitType, EnemyList, std::hash<int>>& GetEnemies() const { return _enemies; }

	};
}


