#pragma once
#include <BWAPI.h>
#include "libs/BWEB/BWEB.h"

namespace KasoBot
{
	class Expansion;
	struct BaseInfo;
	struct EnemyUnit;

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

		//@return next tile where we want to expand
		BWAPI::TilePosition GetNextBase();

		BWAPI::TilePosition GetBuildPosition(BWAPI::UnitType type);

		//@return center pixel for specified building, used for debug drawing and calculating distance to build location
		BWAPI::Position GetCenterOfBuilding(BWAPI::TilePosition pos, BWAPI::UnitType type);

		//@return pointer to unfinished building on specific tile on map
		BWAPI::Unit GetUnfinished(BWAPI::TilePosition pos, BWAPI::UnitType type);

		//create BaseInfo struct for every base
		void ResetBaseInfo(std::vector<std::unique_ptr<BaseInfo>>& output);

		//@return pointer to next base that should be scouted at the start
		const BWEM::Base* NextScoutBaseStart();

		//@return one of the starting areas (except ours) that is closest to this position
		const BWEM::Area* ClosestStart(BWAPI::TilePosition pos);

		//@return position where the scout should move while scouting around the base
		//@param currPos = current position of scouting unit
		BWAPI::Position NextScoutPosition(const BWEM::Area* area, BWAPI::Position currPos);

		//@return position where the default task for armies should be located
		BWAPI::Position DefaultTaskPosition();

		//@return false if building place is visible and building is not longer there
		bool IsStillThere(EnemyUnit& enemy);

		//@return true if any of tiles belonging to base is visible
		bool IsVisible(const BWEM::Base* base);

		//@return true if area is accessible by land from our main
		bool CanAccess(const BWEM::Area* area);
	}
}
