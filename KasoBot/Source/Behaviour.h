#pragma once
#include <BWAPI.h>

namespace KasoBot {

	class Unit;
	class Worker;
	
	//every unit type has it's own behaviour class, these functions are called every frame
	class Behaviour
	{
	protected:

		virtual void Move(BWAPI::Unit unit, BWAPI::Position position);

	public:
		Behaviour();
		virtual ~Behaviour();

		virtual void Work(KasoBot::Worker& unit) { return; }

		virtual void Scout(KasoBot::Unit& unit);
	};
}


