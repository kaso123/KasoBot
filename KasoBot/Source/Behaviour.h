#pragma once
#include <BWAPI.h>

namespace KasoBot {

	class Unit;
	class Worker;
	class Army;
	
	//every unit type has it's own behaviour class, these functions are called every frame
	class Behaviour
	{
	protected:

		virtual void Move(BWAPI::Unit unit, BWAPI::Position position);

		virtual void AttackMove(BWAPI::Unit unit, BWAPI::Position position);

	public:
		Behaviour();
		virtual ~Behaviour();

		virtual void AttackArea(KasoBot::Unit& unit, Army* army);

		virtual void DefendArmy(KasoBot::Unit& unit, Army* army);

		virtual void MoveToArmyCenter(KasoBot::Unit& unit, BWAPI::Position position);

		virtual void Work(KasoBot::Worker& unit) { return; }

		virtual void Scout(KasoBot::Unit& unit);
	};
}


