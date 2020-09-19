#pragma once
#include <BWAPI.h>

namespace KasoBot {
	
	class Worker;
	class Unit;
	class Army;

	class ArmyModule
	{
	private:
		ArmyModule();
		~ArmyModule();
		static ArmyModule* _instance;

		//TODO mockup for workers in army
		std::vector<std::shared_ptr<Worker>> _workers;

		std::vector <std::unique_ptr<Army>> _armies;
	public:
		static ArmyModule* Instance();

		void OnFrame();

		//@param max = max amount of workers needed
		//@return vector of workers from army
		std::vector<std::shared_ptr<Worker>> GetFreeWorkers(size_t max);

		void AddWorker(std::shared_ptr<Worker> worker);

		//@return true if killed worker was from army units
		bool WorkerKilled(BWAPI::Unit unit);

		void AddSoldier(KasoBot::Unit* unit);

		//remove killed soldier from army
		void SoldierKilled(KasoBot::Unit* unit);

		//@return total army supply, excluding mining workers
		int GetArmySupply();

		//move all units that are standing on this tile to unblock construction
		void ClearTiles(BWAPI::TilePosition pos, BWAPI::UnitType type);

		//@return true if no worker is available to scout
		bool NeedScout();

		//getters and setters

		const std::vector <std::unique_ptr<Army>>& Armies() const { return _armies; }
		const std::vector<std::shared_ptr<Worker>>& Workers() const { return _workers; }

	};
}


