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

	class Army
	{
	private:
		std::vector <KasoBot::Unit*> _soldiers;
		Task* _task;
		std::unique_ptr<Armies::Box> _box; //bounding box around all units

		//calculate center point between all units
		void CalculateCenter();

		//check if task is not finished and remove it if needed
		void CheckTask();

	public:
		Army();
		~Army();

		void OnFrame();

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
		void RemoveTask();

		//@return current task or default task if none
		Task* Task();

		//getters and setters

		const Armies::Box& BoundingBox() { return *_box; }
		const std::vector<KasoBot::Unit*>& Units() const { return _soldiers; }
	};

}
