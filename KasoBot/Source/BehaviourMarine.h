#pragma once
#include <BWAPI.h>
#include "Behaviour.h"

namespace KasoBot {

	class BehaviourMarine : public Behaviour
	{
	private:
		void GoInBunker(BWAPI::Unit unit, BWAPI::Unit bunker);

		void LeaveBunker(BWAPI::Unit unit, BWAPI::Unit bunker);

	public:
		BehaviourMarine();
		~BehaviourMarine();

		void DefendArmy(KasoBot::Unit& unit, Army* army) override;

		void HoldPosition(KasoBot::Unit& unit, Army* army) override;

		void AttackArea(KasoBot::Unit& unit, Army* army) override;

		void ScoutArea(KasoBot::Unit& unit, Army* army) override;

	};
}
