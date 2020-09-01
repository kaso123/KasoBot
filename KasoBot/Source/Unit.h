#pragma once
#include <BWAPI.h>


namespace KasoBot {

	class Behaviour;

	class Unit
	{
	protected:
		BWAPI::Unit _pointer;
		std::unique_ptr<Behaviour> _behaviour;

		bool _playerControl;
		int _playerControlFrame;
	public:
		Unit(BWAPI::Unit unit);
		virtual ~Unit();

		//equivalent to onFrame for army units, including workers that are in the army
		virtual void Fight();

		//equivalent to onFrame used in worker class (not for workers in army )
		virtual void Work() { return; }

		//switch between AI controlled and player controlled behaviour
		void ChangeDebugControl();
		
		//getters and setters

		BWAPI::Unit GetPointer() const { return _pointer; }
		bool PlayerControlled() const { return _playerControl; }
	};
}


