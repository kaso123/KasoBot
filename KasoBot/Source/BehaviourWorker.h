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

		//assign order to worker
		//@target = mineral patch or refinery
		void Gather(BWAPI::Unit unit, BWAPI::Unit target);


	public:
		BehaviourWorker();
		~BehaviourWorker();

		//logic for mining minerals, building and repairing buildings
		void Work(Worker& worker) override;
	};
}


