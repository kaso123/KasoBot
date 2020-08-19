#include "MapModule.h"

using namespace KasoBot;

BWEB::Station* Map::GetStation(BWAPI::TilePosition pos)
{
	return BWEB::Stations::getClosestStation(pos);
}

void Map::Global::Initialize()
{
	BWEB::Map::mapBWEM.Initialize(BWAPI::BroodwarPtr);
	BWEB::Map::mapBWEM.EnableAutomaticPathAnalysis();
	BWEB::Map::mapBWEM.FindBasesForStartingLocations();
	BWEB::Map::onStart();
	BWEB::Stations::findStations();
}
