#pragma once
#include <BWAPI.h>
#include "Behaviour.h"

namespace BWEM {
	class Base;
}

namespace KasoBot {

	class EnemyArmy;

	class BehaviourWraith : public Behaviour
	{
	private:
		//check if it is needed to cloak / uncloak
		//@return true if command was issued
		bool HandleCloak(KasoBot::Unit& unit);

		//check if we want to disengage from fight
		//@return true if command was issued
		bool HandleDisengage(KasoBot::Unit& unit);

		//try to move away from enemy army
		void Disengage(KasoBot::Unit& unit, EnemyArmy& army);
	public:
		BehaviourWraith();
		~BehaviourWraith();

		void HarassArea(KasoBot::Unit& unit, Army* army) override;
		void SupportArmy(KasoBot::Unit& unit, Army* army) override;
	};
}

