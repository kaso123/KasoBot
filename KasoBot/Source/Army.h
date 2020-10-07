#pragma once
#include <BWAPI.h>

namespace KasoBot {

	class Unit;
	class Task;

	class Army
	{
	private:
		std::vector <KasoBot::Unit*> _soldiers;
		Task* _task;
	public:
		Army();
		~Army();

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

		//getters and setters

		Task* Task() { return _task; }
	};

}
