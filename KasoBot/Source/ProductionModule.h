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

		int _reservedMinerals;
		int _reservedGas;

	public:
		static ProductionModule* Instance();

		void AddUnit(BWAPI::Unit unit);
		void AddBuilding(BWAPI::Unit unit);

		void RemoveUnit(BWAPI::Unit unit);
		void RemoveBuilding(BWAPI::Unit unit);

		//find building that can build this addon
		//@return true if building found and addon started
		bool BuildAddon(BWAPI::UnitType type);

		//create productionItem and send it right to workersModule, use for debugging purposes only
		void DebugBuild(BWAPI::UnitType type);

		//add resources for this type to reserved
		void ReserveResources(BWAPI::UnitType type);

		//subtract this unit's cost from reserved resources
		void FreeResources(BWAPI::UnitType type);

		//getters and setters

		const std::vector<std::unique_ptr<ProductionItem>>& GetItems() { return _items; }
		const int GetReservedMinerals() { return _reservedMinerals; }
		const int GetReservedGas() { return _reservedGas; }
	};
}


