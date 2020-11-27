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

		int _antiAir; //number of units that can shoot air

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
		//@return true if joining successful
		bool Join(EnemyArmy* toJoin);

		//remove all units from this army, only should be used when joining two armies
		void ClearUnits();

		//@return true if army is close to our buildings
		bool IsThreat();

		//@return true if there are cannons and pylons in army
		bool IsCannonRush();

		//@return true if there are many workers in our base
		bool IsWorkerRush();

		//@return true if there are only observers / overlords in army
		bool IsFlyingScout();

		//@return true if all enemies in this army are flying
		bool IsOnlyFlying();

		//@return total supply of this army
		int Supply();

		//getters and setters

		const Armies::Box& BoundingBox() { return *_box; }
		const std::vector<EnemyUnit*>& Units() const { return _units; }
		int AntiAirCount() const { return _antiAir; }
	};
}



