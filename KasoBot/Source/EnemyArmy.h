#pragma once
#include <BWAPI.h>

namespace KasoBot {

	namespace Armies { struct Box; }
	struct EnemyUnit;

	class EnemyArmy
	{
	private:
		std::unique_ptr<Armies::Box> _box;
		std::vector<EnemyUnit*> _units;

		//calculate center point between all units
		void CalculateCenter();

		//check all units and remove those that are too far from center
		void CheckUnits();

	public:
		EnemyArmy();
		~EnemyArmy();

		void OnFrame();

		void AddEnemy(EnemyUnit* unit);

		void RemoveEnemy(EnemyUnit* unit);

		//move all units from one army to this one
		void Join(EnemyArmy* toJoin);

		//remove all units from this army, only should be used when joining two armies
		void ClearUnits();

		//getters and setters

		const Armies::Box& BoundingBox() { return *_box; }
		const std::vector<EnemyUnit*>& Units() const { return _units; }
	};
}



