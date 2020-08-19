#pragma once
#include <BWAPI.h>
#include "libs/BWEB/BWEB.h"

namespace BWEB {
	class Station;
}
namespace KasoBot
{
	namespace Map 
	{
		namespace Global {
			void Initialize();
		}
		//@return closest BWEB::Station to this position
		BWEB::Station* GetStation(BWAPI::TilePosition pos);
	}
	
}
