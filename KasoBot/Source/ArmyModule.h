#pragma once
#include <BWAPI.h>

namespace BWEM { class Area; }

namespace KasoBot {
	
	namespace Tasks { enum Type; }
	class Worker;
	class Unit;
	class Army;
	class WorkerArmy;
	class Task;
	class EnemyArmy;

	class ArmyModule
	{
	private:
		ArmyModule();
		~ArmyModule();
		static ArmyModule* _instance;

		std::unique_ptr<WorkerArmy> _workers; //workers in army

		std::vector <std::unique_ptr<Army>> _armies;

		std::vector <std::unique_ptr<Task>> _tasks;

		std::unique_ptr<Task> _defaultTask; //task that army does when it can't do other tasks

		//cycle through tasks and try to assign each to an army
		void AssignTasks();

		//check if we have enough attack tasks and create more if not
		void CreateAttackTasks();
		
		//check if we have enough scout tasks and create more if not
		void CreateScoutTasks();

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

		//create new attackArea task if it doesn't exist already
		//@param limit = how many attack tasks for the same area we want, default 1
		bool AddAttackTask(const BWEM::Area* area, int limit = 1);

		//create new defendArmy task if it doesn't exist already
		bool AddDefendTask(EnemyArmy* enemy);

		//create new holdPosition task if it doesn't exist already
		bool AddHoldTask(BWAPI::Position pos);

		//create new scoutArea task if it doesn't exist already
		bool AddScoutTask(const BWEM::Area* area);

		//find task for this army and remove it
		void EnemyArmyRemoved(EnemyArmy* enemy);

		//check if default task should be changed
		void ResetDefaultTask();
		
		//getters and setters

		const std::vector<std::unique_ptr<Army>>& Armies() const { return _armies; }
		const std::unique_ptr<WorkerArmy>& WorkerArmy() const { return _workers; }
		const std::vector<std::unique_ptr<Task>>& Tasks() const { return _tasks; }
		Task* DefaultTask() const { return _defaultTask.get(); }
	};
}


