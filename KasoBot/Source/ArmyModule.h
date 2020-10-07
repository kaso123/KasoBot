#pragma once
#include <BWAPI.h>

namespace BWEM { class Area; }

namespace KasoBot {
	
	namespace Tasks { enum Type; }
	class Worker;
	class Unit;
	class Army;
	class Task;

	class ArmyModule
	{
	private:
		ArmyModule();
		~ArmyModule();
		static ArmyModule* _instance;

		//TODO mockup for workers in army
		std::vector<std::shared_ptr<Worker>> _workers;

		std::vector <std::unique_ptr<Army>> _armies;

		std::vector <std::unique_ptr<Task>> _tasks;

		//cycle through tasks and try to assign each to an army
		void AssignTasks();

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

		//create new task if it doesn't exist already
		void AddTask(Tasks::Type type, BWAPI::Position pos);

		//create new task if it doesn't exist already
		void AddTask(Tasks::Type type, const BWEM::Area* area);
		
		//getters and setters

		const std::vector <std::unique_ptr<Army>>& Armies() const { return _armies; }
		const std::vector<std::shared_ptr<Worker>>& Workers() const { return _workers; }

	};
}


