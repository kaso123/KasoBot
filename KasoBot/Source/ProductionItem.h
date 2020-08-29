#pragma once
#include <BWAPI.h>

namespace KasoBot {

	namespace Production {
		enum State {
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
		~ProductionItem();


		//setters and getters

		BWAPI::TilePosition GetLocation() const { return _buildLocation; }
		void SetLocation(BWAPI::TilePosition pos) { _buildLocation = pos; }
		Production::State GetState() const { return _state; }
	};
}


