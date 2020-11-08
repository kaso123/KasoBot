#pragma once
#include <BWAPI.h>

namespace KasoBot {

	namespace Armies {
		struct Box {
			BWAPI::Position _topLeft;
			BWAPI::Position _bottomRight;
			BWAPI::Position _center;
		};
	}
	class Unit;
	class Task;
	class Worker;

	class Army
	{
	protected:
		std::vector <KasoBot::Unit*> _soldiers;
		Task* _task;
		std::unique_ptr<Armies::Box> _box; //bounding box around all units

		//calculate center point between all units
		void CalculateCenter();

		//check if task is not finished and remove it if needed
		virtual void CheckTask();

	public:
		Army();
		virtual ~Army();

		virtual void OnFrame();

		//add new soldier, army can decline unit for various reasons
		//@return true if soldier was added to this army
		bool AddSoldier(KasoBot::Unit* unit);

		//remove killed soldier from army
		//@return true if soldier was from this army
		bool SoldierKilled(KasoBot::Unit* unit);

		int GetSupply();

		//move all units that are standing on this tile to unblock construction
		void ClearTiles(BWAPI::TilePosition pos, BWAPI::UnitType type);

		//assign new task and overwrite previous task
		void AssignTask(Task* task);

		//remove pointer to task, if any
		virtual void RemoveTask();

		//@return current task or default task if none
		Task* Task();

		//@return unit that is suitable for scouting, nullptr if none in army
		KasoBot::Unit* GetScoutSoldier();

		//@return unit that should be repaired by SCVs
		virtual BWAPI::Unit GetRepairTarget();

		//getters and setters

		const Armies::Box& BoundingBox() { return *_box; }
		const std::vector<KasoBot::Unit*>& Units() const { return _soldiers; }
	};

	class WorkerArmy : public Army {
	private:
		std::vector<std::shared_ptr<Worker>> _workers;

		void CheckTask() override;
	public:
		WorkerArmy();
		~WorkerArmy();

		//@param max = max amount of workers needed
		//@return vector of workers from army
		std::vector<std::shared_ptr<Worker>> GetFreeWorkers(size_t max);

		void AddWorker(std::shared_ptr<Worker> worker);

		//@return true if killed worker was from army units
		bool WorkerKilled(BWAPI::Unit unit);

		void OnFrame() override;

		//remove task and send workers back to workersModule
		void RemoveTask() override;

		BWAPI::Unit GetRepairTarget() override;

		//getters and setters

		const std::vector<std::shared_ptr<KasoBot::Worker>>& Workers() const { return _workers; }
	};
}
