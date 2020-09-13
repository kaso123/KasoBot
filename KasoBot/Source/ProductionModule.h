#pragma once
#include <BWAPI.h>

namespace KasoBot {
	class Unit;
	class ProductionItem;
	class Expansion;
}

typedef std::vector<std::unique_ptr<KasoBot::Unit>> UnitList;

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

		//order new supply depot when getting close to supply block
		void PreventSupplyBlock();

	public:
		static ProductionModule* Instance();

		void OnFrame();

		void AddUnit(BWAPI::Unit unit);
		void AddBuilding(BWAPI::Unit unit);

		void RemoveUnit(BWAPI::Unit unit);
		void RemoveBuilding(BWAPI::Unit unit);

		//find building that can build this addon
		//@return true if building found and addon started
		bool BuildAddon(BWAPI::UnitType type);

		//create new productionItem and assign worker
		//@return true if worker was assigned
		bool BuildBuilding(BWAPI::UnitType type);

		//build refinery at the closest position to this base (should be its vespene geyser, but can't guarantee)
		bool BuildRefineryAtExpansion(Expansion& exp);

		//find building that builds this unit and build it
		//@return true if unit was started
		bool BuildUnit(BWAPI::UnitType type);

		//create productionItem and send it right to workersModule, use for debugging purposes only
		void DebugBuild(BWAPI::UnitType type);

		//add resources for this type to reserved
		void ReserveResources(BWAPI::UnitType type);

		//subtract this unit's cost from reserved resources
		void FreeResources(BWAPI::UnitType type);

		//check if we can build this unit considering reserved resources also
		//@return true if unit can be built
		bool CheckResources(BWAPI::UnitType type);

		//@return true if we have enough resources to assign worker to build
		bool CanSendWorker(BWAPI::UnitType type);

		//try to make this new item
		//@return true if successfuly started new task
		bool NewTask(BWAPI::UnitType type);

		//@return true if this building is in queue (in any state except DONE)
		bool IsInQueue(BWAPI::UnitType type);

		//@return number of units/buildings of this type (including in progress buildings)
		int GetCountOf(BWAPI::UnitType type);

		//getters and setters

		const std::vector<std::unique_ptr<ProductionItem>>& GetItems() { return _items; }
		const int GetReservedMinerals() { return _reservedMinerals; }
		const int GetReservedGas() { return _reservedGas; }
		const std::unordered_map<BWAPI::UnitType, UnitList, std::hash<int>>& Units() { return _unitList; }
		const std::unordered_map<BWAPI::UnitType, UnitList, std::hash<int>>& Buildings() { return _buildingList; }
	};
}


