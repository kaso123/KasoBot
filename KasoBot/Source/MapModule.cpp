#include "MapModule.h"
#include "Expansion.h"
#include "Config.h"
#include "WorkersModule.h"
#include "BaseInfo.h"

#include <math.h>

#define PI 3.141593
#define TILE_SIZE 32
#define DIST_LIMIT 100
#define SCOUT_ANGLE_INCREASE 0.35

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
			dist = (int)loc.getDistance(pos);
			area = BWEM::Map::Instance().GetNearestArea(loc);
		}
	}
	return area;
}

BWAPI::Position Map::NextScoutPosition(const BWEM::Area * area, BWAPI::Position currPos)
{
	if (area->Bases().empty())
		return BWAPI::Positions::Invalid;

	//calculate angle
	int originX = area->Bases().front().Center().x;
	int originY = area->Bases().front().Center().y;
	int radius = (int)currPos.getDistance(area->Bases().front().Center());

	float angle = acos((currPos.x - originX) / (float)radius);
	
	if (currPos.y < originY) //arccos only returns values <0,PI>, upper half of circle
		angle += 2 * ((float)PI - angle);

	radius = Config::Units::ScoutBaseRadius();

	//calculate point
	int newX = originX + int(radius * cos(angle));
	int newY = originY + int(radius * sin(angle));
	newX = std::clamp(newX, TILE_SIZE, (BWAPI::Broodwar->mapWidth() - 2) * TILE_SIZE);
	newY = std::clamp(newY, TILE_SIZE, (BWAPI::Broodwar->mapHeight() - 2) * TILE_SIZE);

	BWAPI::Position point = BWAPI::Position(newX, newY);
	if (point.getDistance(currPos) > DIST_LIMIT)
		return point;

	//get next angle on circle
	angle += (float)SCOUT_ANGLE_INCREASE;
	if (angle > 2 * PI)
		angle -= 2* (float)PI;

	//calculate point
	newX = originX + int(radius * cos(angle));
	newY = originY + int(radius * sin(angle));
	newX = std::clamp(newX, TILE_SIZE, (BWAPI::Broodwar->mapWidth() - 2) * TILE_SIZE);
	newY = std::clamp(newY, TILE_SIZE, (BWAPI::Broodwar->mapHeight() - 2) * TILE_SIZE);

	point = BWAPI::Position(newX, newY);
		return point;
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