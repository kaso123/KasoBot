#pragma once
#include <BWAPI.h>
#include "Behaviour.h"
#include "Config.h"

namespace KasoBot {

	class Worker;

	class BehaviourWorker : public Behaviour
	{
	private:

		//mining minerals behaviour
		void Minerals(Worker& worker);

		//mining gas behaviour
		void Gas(Worker& worker);

		//move to build location or to started building if finishing build
		void MoveToBuild(Worker& worker);

		//construct building
		void Construct(Worker& worker);

		//assign gather order to worker
		//@target = mineral patch
		void GatherMinerals(BWAPI::Unit unit, BWAPI::Unit target);
		
		//assign gather order to worker
		//@target = refinery
		void GatherGas(BWAPI::Unit unit, BWAPI::Unit target);

		//assign return cargo order to worker
		void ReturnCargo(BWAPI::Unit unit);

		//assign build order to worker
		void Build(BWAPI::Unit unit, BWAPI::TilePosition pos, BWAPI::UnitType type);



	public:
		BehaviourWorker();
		~BehaviourWorker();

		//logic for mining minerals, building and repairing buildings
		void Work(Worker& worker) override;
	};
}


