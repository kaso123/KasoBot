#pragma once
#include <BWAPI.h>


namespace KasoBot {

	namespace Units {
		enum Role {
			IDLE,
			SCOUT
		};
	}
	class Behaviour;
	class Task;
	class Army;

	class Unit
	{
	protected:
		BWAPI::Unit _pointer;
		std::unique_ptr<Behaviour> _behaviour;

		Units::Role _role;
		bool _playerControl;
		bool _lock; //used for buildings that needs addons to lock them from training more units before addon is started
		int _clearTileLock; //used for units that are moving away from build location to make it accessible (to not overwrite for few seconds)
		int _playerControlFrame;
	public:
		Unit(BWAPI::Unit unit);
		virtual ~Unit();

		//equivalent to onFrame for army units, including workers that are in the army
		virtual void Fight(Army* army);

		//equivalent to onFrame used in worker class (not for workers in army )
		virtual void Work() { return; }

		//equivalent to onFrame for any scouting unit
		virtual void Scout();

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
		const Units::Role GetRole() const { return _role; }
		void SetRole(Units::Role role) { _role = role; }
	};
}


