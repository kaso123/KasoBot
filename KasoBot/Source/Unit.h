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
		bool _lock; //used for buildings that needs addons to lock them from training more units before addon is started
		int _clearTileLock; //used for units that are moving away from build location to make it accessible (to not overwrite for few seconds)
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
		
		//move away from current tile and ignore other commands for a while
		void ClearTile();

		//getters and setters

		BWAPI::Unit GetPointer() const { return _pointer; }
		bool PlayerControlled() const { return _playerControl; }
		void Lock() { _lock = true; }
		void Unlock() { _lock = false; }
		bool IsLocked() const { return _lock; }
	};
}


