#pragma once
#include <BWAPI.h>
#include "libs/BWEB/BWEB.h"

namespace BWEB {
	class Station;
}
namespace KasoBot
{
	class Expansion;
	struct BaseInfo;

	namespace Map 
	{
		namespace Global {
			void Initialize();
		}
		//@return closest BWEB::Station to this position
		BWEB::Station* GetStation(BWAPI::TilePosition pos);

		//minerals are assigned closest first, when every mineral is assigned, start mining from furthest
		//@return pointer to next mineral in expansion that should be assigned to worker
		BWEM::Mineral* NextMineral(const BWEM::Base* base);

		BWAPI::TilePosition GetBuildPosition(BWAPI::UnitType type);

		//@return center pixel for specified building, used for debug drawing and calculating distance to build location
		BWAPI::Position GetCenterOfBuilding(BWAPI::TilePosition pos, BWAPI::UnitType type);

		//@return pointer to unfinished building on specific tile on map
		BWAPI::Unit GetUnfinished(BWAPI::TilePosition pos, BWAPI::UnitType type);

		//create BaseInfo struct for every base
		void ResetBaseInfo(std::vector<std::unique_ptr<BaseInfo>>& output);

		//@return pointer to next area that should be scouted
		const BWEM::Base* NextScoutBase();
	}
	
}
