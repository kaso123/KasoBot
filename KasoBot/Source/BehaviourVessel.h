#pragma once
#include <BWAPI.h>
#include "Behaviour.h"

namespace KasoBot {

	class BehaviourVessel : public Behaviour
	{
	private:
		void AttackMove(BWAPI::Unit unit, BWAPI::Position position, bool reset = false) override;
	public:
		BehaviourVessel();
		~BehaviourVessel();

		void AttackArea(KasoBot::Unit& unit, Army* army) override;

		void DefendArmy(KasoBot::Unit& unit, Army* army) override;
	};
}


