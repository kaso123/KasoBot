#pragma once
#include <BWAPI.h>
#include "Behaviour.h"

namespace KasoBot {

	class BehaviourMedic : public Behaviour
	{
	private:

		void AttackMove(BWAPI::Unit unit, BWAPI::Position position) override;

	public:
		BehaviourMedic();
		~BehaviourMedic();

		void AttackArea(KasoBot::Unit& unit, Army* army) override;
	};
}

