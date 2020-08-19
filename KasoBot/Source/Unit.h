#pragma once
#include <BWAPI.h>


namespace KasoBot {
	class Unit
	{
	private:
		BWAPI::Unit _pointer;
	public:
		Unit(BWAPI::Unit unit);
		virtual ~Unit();


		
		//getters and setters

		BWAPI::Unit GetPointer() { return _pointer; };
	};
}


