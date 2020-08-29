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

		//assign gather order to worker
		//@target = mineral patch
		void GatherMinerals(BWAPI::Unit unit, BWAPI::Unit target);
		
		//assign gather order to worker
		//@target = refinery
		void GatherGas(BWAPI::Unit unit, BWAPI::Unit target);

		
		//assign return cargo order to worker
		void ReturnCargo(BWAPI::Unit unit);



	public:
		BehaviourWorker();
		~BehaviourWorker();

		//logic for mining minerals, building and repairing buildings
		void Work(Worker& worker) override;
	};
}


