#pragma once
#include <BWAPI.h>
#include "libs/BWEB/BWEB.h"

namespace BWEB {
	class Station;
}
namespace KasoBot
{
	class Expansion;

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
	}
	
}
