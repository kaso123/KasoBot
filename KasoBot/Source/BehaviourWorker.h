#pragma once
#include <BWAPI.h>
#include "Behaviour.h"

namespace KasoBot {
	class BehaviourWorker : public Behaviour
	{
	private:
	public:
		BehaviourWorker();
		~BehaviourWorker();

		//logic for mining minerals, building and repairing buildings
		void Work(KasoBot::Unit* unit) override;
	};
}


