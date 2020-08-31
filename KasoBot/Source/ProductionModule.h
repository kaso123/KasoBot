#pragma once
#include <BWAPI.h>

namespace KasoBot {
	class Unit;
	class ProductionItem;
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

		std::vector<std::unique_ptr<ProductionItem>> _items;

	public:
		static ProductionModule* Instance();

		void AddUnit(BWAPI::Unit unit);
		void AddBuilding(BWAPI::Unit unit);

		void RemoveUnit(BWAPI::Unit unit);
		void RemoveBuilding(BWAPI::Unit unit);

		//create productionItem and send it right to workersModule, use for debugging purposes only
		void DebugBuild(BWAPI::UnitType type);
	};
}


