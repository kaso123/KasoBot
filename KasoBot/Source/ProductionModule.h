#pragma once
#include <BWAPI.h>

namespace KasoBot {
	class Unit;
}

typedef std::vector<std::shared_ptr<KasoBot::Unit>> UnitList;

namespace KasoBot {
	class ProductionModule
	{
	private:
		ProductionModule();
		~ProductionModule();
		static ProductionModule* _instance;

		std::unordered_map<BWAPI::UnitType, UnitList, std::hash<int>> _unitList;
		std::unordered_map<BWAPI::UnitType, UnitList, std::hash<int>> _buildingList;

	public:
		static ProductionModule* Instance();

		void AddUnit(BWAPI::Unit unit);
		void AddBuilding(BWAPI::Unit unit);
	};
}


