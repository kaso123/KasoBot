#pragma once
#include <BWAPI.h>
#include "Behaviour.h"

namespace BWEM {
	class Base;
}

namespace KasoBot {

	class BehaviourWraith : public Behaviour
	{
	private:

	public:
		BehaviourWraith();
		~BehaviourWraith();

		void HarassArea(KasoBot::Unit& unit, Army* army) override;
	};
}

