#include "MapModule.h"
#include "Expansion.h"
#include "Config.h"

using namespace KasoBot;

BWEB::Station* Map::GetStation(BWAPI::TilePosition pos)
{
	return BWEB::Stations::getClosestStation(pos);
}

BWEM::Mineral* Map::NextMineral(const BWEM::Base* base)
{
	_ASSERT(base);
	if (base->Minerals().empty())
		return nullptr;

	BWEM::Mineral* mineral = nullptr;
	int dist = INT_MAX;
	
	for (auto patch : base->Minerals()) //one worker for every mineral patch first
	{
		if (!patch->Unit() || patch->Amount() <= 0)
			continue;

		int currDist = patch->Unit()->getDistance(base->Center());
		if ((!mineral ||  currDist < dist) && patch->Data() == 0) {
			mineral = patch;
			dist = currDist;
		}
	}

	if (mineral)
		return mineral;

	//worker is already on every patch
	for (int i = 1; i < Config::Workers::MaxPerMineral(); i++)
	{
		dist = 0; //start saturating from the farthest mineral patch
		for (auto patch : base->Minerals())
		{
			if (!patch->Unit() || patch->Amount() <= 0)
				continue;

			int currDist = patch->Unit()->getDistance(base->Center());
			if ((!mineral || currDist > dist) && patch->Data() == i) {
				mineral = patch;
				dist = currDist;
			}
		}

		if (mineral)
			return mineral;
	}
	
	return nullptr;
}

void Map::Global::Initialize()
{
	BWEB::Map::mapBWEM.Initialize(BWAPI::BroodwarPtr);
	BWEB::Map::mapBWEM.EnableAutomaticPathAnalysis();
	BWEB::Map::mapBWEM.FindBasesForStartingLocations();
	BWEB::Map::onStart();
	BWEB::Stations::findStations();
}
