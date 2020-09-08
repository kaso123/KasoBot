#pragma once
#include <BWAPI.h>

namespace KasoBot {

	namespace Production {
		enum State {
			UNFINISHED,
			WAITING,
			ASSIGNED,
			BUILDING,
			DONE
		};
	}

	class ProductionItem
	{
	private:
		BWAPI::UnitType _type;
		BWAPI::TilePosition _buildLocation;
		Production::State _state;

	public:
		ProductionItem(BWAPI::UnitType type);
		ProductionItem(BWAPI::UnitType type, BWAPI::TilePosition pos);
		~ProductionItem();

		//set item to be in assigned state
		void Assigned();

		//set production item to be in building state
		void BuildStarted();

		//reset to assigned state, something happened
		void Restart();

		//set state to done
		void Finish();

		//called when worker died
		void WorkerDied();

		//called when building that should be built in this Item was destroyed
		void BuildingDestroyed();

		//setters and getters

		BWAPI::TilePosition GetLocation() const { return _buildLocation; }
		void SetLocation(BWAPI::TilePosition pos) { _buildLocation = pos; }
		Production::State GetState() const { return _state; }
		BWAPI::UnitType GetType() const { return _type; }
	};
}


