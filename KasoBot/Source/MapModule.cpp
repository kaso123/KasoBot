#include "MapModule.h"
#include "Expansion.h"
#include "Config.h"
#include "WorkersModule.h"
#include "BaseInfo.h"

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

BWAPI::TilePosition Map::GetBuildPosition(BWAPI::UnitType type)
{
	if (type.isResourceDepot()) //command center
	{
		//TODO get next base to expand to
		//now only getting closest base without CC
		int closestDist = INT_MAX;
		BWAPI::TilePosition closest = BWAPI::TilePositions::Invalid;
		for (auto station : BWEB::Stations::getStations())
		{
			if (BWEB::Map::isUsed(station.getBWEMBase()->Location()) != BWAPI::UnitTypes::None)
				continue;

			int dist = (int)station.getBWEMBase()->Location().getDistance(BWAPI::Broodwar->self()->getStartLocation());
			if (dist < closestDist)
			{
				closestDist = dist;
				closest = station.getBWEMBase()->Location();
			}
		}
		return  closest;
	}
	else if (type.isRefinery())
	{
		//find expansion that needs refinery
		for (auto& exp : WorkersModule::Instance()->ExpansionList())
		{
			if (exp->GetRefinery())
				continue;

			return (BWAPI::Broodwar->getBuildLocation(type, exp->GetStation()->getBWEMBase()->Location()));
		}
		//TODO all bases have refinery assigned
		return (BWAPI::Broodwar->getBuildLocation(type, BWAPI::Broodwar->self()->getStartLocation()));
	}
	else if (type == BWAPI::UnitTypes::Terran_Missile_Turret)
	{
		//TODO find expansion that needs turrets
		return BWEB::Map::getDefBuildPosition(type);
	}

	//get build positions from BWEB
	return BWEB::Map::KasoBot::GetBuildPosition(type);
}

BWAPI::Position Map::GetCenterOfBuilding(BWAPI::TilePosition pos, BWAPI::UnitType type)
{
	return BWAPI::Position(pos + BWAPI::TilePosition(type.tileWidth()/2, type.tileHeight()/2));
}

BWAPI::Unit Map::GetUnfinished(BWAPI::TilePosition pos, BWAPI::UnitType type)
{
	auto set = BWAPI::Broodwar->getUnitsOnTile(pos);
	for (auto unit : set)
	{
		if (unit->getType() == type && !unit->isCompleted())
			return unit;
	}

	_ASSERT(false);
	return nullptr;
}

void Map::ResetBaseInfo(std::vector<std::unique_ptr<BaseInfo>>& output)
{
	for (auto& area : BWEM::Map::Instance().Areas())
	{
		for (auto& base : area.Bases())
		{
			base.SetPtr(output.emplace_back(std::make_unique<BaseInfo>()).get());
		}
	}

	((BaseInfo*)BWEB::Map::getMainArea()->Bases().front().Ptr())->_owner = Base::Owner::PLAYER;
}

const BWEM::Base* Map::NextScoutBase()
{
	//cycle start locations, skip own
	for (auto& loc : BWEM::Map::Instance().StartingLocations())
	{
		if (loc == BWAPI::Broodwar->self()->getStartLocation())
			continue;

		auto area = BWEM::Map::Instance().GetNearestArea(loc);
		if (!area || area->Bases().empty())
			continue;

		for (auto &base : area->Bases())
		{
			if (((BaseInfo*)base.Ptr())->_owner == Base::Owner::UNKNOWN)
			{
				return &base;
			}
		}
	}
	return nullptr;
}

const BWEM::Area* Map::ClosestStart(BWAPI::TilePosition pos)
{
	const BWEM::Area* area = nullptr;
	int dist = INT_MAX;
	//cycle start locations, skip own
	for (auto& loc : BWEM::Map::Instance().StartingLocations())
	{
		if (loc == BWAPI::Broodwar->self()->getStartLocation())
			continue;

		if (loc.getDistance(pos) < dist)
		{
			dist = loc.getDistance(pos);
			area = BWEM::Map::Instance().GetNearestArea(loc);
		}
	}
	return area;
}

void Map::Global::Initialize()
{
	BWEB::Map::mapBWEM.Initialize(BWAPI::BroodwarPtr);
	BWEB::Map::mapBWEM.EnableAutomaticPathAnalysis();
	BWEB::Map::mapBWEM.FindBasesForStartingLocations();
	BWEB::Map::onStart();
	BWEB::Stations::findStations();
	BWEB::Blocks::findBlocks();
}