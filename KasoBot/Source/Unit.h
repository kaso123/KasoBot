#pragma once
#include <BWAPI.h>


namespace KasoBot {

	class Behaviour;

	class Unit
	{
	protected:
		BWAPI::Unit _pointer;
		std::unique_ptr<Behaviour> _behaviour;
	public:
		Unit(BWAPI::Unit unit);
		virtual ~Unit();

		//equivalent to onFrame for army units, including workers that are in the army
		virtual void Fight();

		//equivalent to onFrame used in worker class (not for workers in army )
		virtual void Work() { return; }


		
		//getters and setters

		BWAPI::Unit GetPointer() { return _pointer; };
	};
}


