#pragma once
#include <BWAPI.h>

namespace KasoBot {

	class Unit;
	
	//every unit type has it's own behaviour class, these functions are called every frame
	class Behaviour
	{
	protected:
	public:
		Behaviour();
		virtual ~Behaviour();

		virtual void Work(KasoBot::Unit* unit) { return; }
	};
}


