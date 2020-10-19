#include "MapModule.h"
#include "Expansion.h"
#include "Config.h"
#include "WorkersModule.h"
#include "ArmyModule.h"
#include "ScoutModule.h"
#include "BaseInfo.h"
#include "Task.h"

#include <math.h>

#define PI 3.141593
#define TILE_SIZE 32
#define SCOUT_ANGLE_INCREASE 0.2

using namespace KasoBot;

namespace {

	//check calculated point, make sure it is not blocked and is in same area as the base
	BWAPI::Position GetSafeScoutPoint(BWAPI::Position oldPoint, BWAPI::Position base)
	{
		auto area = BWEM::Map::Instance().GetArea(BWAPI::TilePosition(base));
		_ASSERT(area);

		auto tile = BWAPI::TilePosition(oldPoint);
		//check area
		
		int counter = 0; //safe check
		bool outOfArea = false; //set to true, when generated point was already outside of area (then we always move the point closer to base)
		while (counter < 200) //TODO make configurable
		{
			counter++;
			if (tile.isValid() && BWEM::Map::Instance().GetArea(tile) != area)
			{
				outOfArea = true;
				//move point closer
				auto vector = base - oldPoint;

				//normalized vector
				float normalLength = sqrt((float)vector.x * (float)vector.x + (float)vector.y * (float)vector.y);
				float normalized[2] = { vector.x / normalLength, vector.y / normalLength };
				
				//move closer on vector
				oldPoint = BWAPI::Position((int)(oldPoint.x + 10 * normalized[0]), (int)(oldPoint.y + 10 * normalized[1])); //TODO make configurable
				tile = BWAPI::TilePosition(oldPoint);
				continue;
			}

			if (!tile.isValid())
				outOfArea = true;

			if (tile.isValid() && BWEB::Map::isWalkable(tile) && BWEB::Map::isUsed(tile) == BWAPI::UnitTypes::None)
				return BWAPI::Position(tile) + BWAPI::Position(16,16);

			//try to go outwards first
			if (!outOfArea)
			{
				//move point further away from base
				auto vector = base - oldPoint;

				//normalized vector
				float normalLength = sqrt((float)vector.x * (float)vector.x + (float)vector.y * (float)vector.y);
				float normalized[2] = { vector.x / normalLength, vector.y / normalLength };

				//move further on vector
				oldPoint = BWAPI::Position((int)(oldPoint.x - 10 * normalized[0]), (int)(oldPoint.y - 10 * normalized[1])); //TODO make configurable
				tile = BWAPI::TilePosition(oldPoint);
				continue;
			}
			else //else go closer
			{
				//move point closer
				auto vector = base - oldPoint;

				//normalized vector
				float normalLength = sqrt((float)vector.x * (float)vector.x + (float)vector.y * (float)vector.y);
				float normalized[2] = { vector.x / normalLength, vector.y / normalLength };

				//move closer on vector
				oldPoint = BWAPI::Position((int)(oldPoint.x + 10 * normalized[0]), (int)(oldPoint.y + 10 * normalized[1])); //TODO make configurable
				tile = BWAPI::TilePosition(oldPoint);

				if (oldPoint.getDistance(base) < 30) //TODO make configurable
					return base;

				continue;
			}
		}
		
		//couldn't find suitable point, just move towards base
		return base;
		
	}
}

BWEB::Station* Map::GetStation(BWAPI::TilePosition pos)
{
	return BWEB::Stations::getClosestStation(pos);
}

BWEM::Mineral* Map::NextMineral(const BWEM::Base* base)
{
	_ASSERT(base);
	_ASSERT(!base->Minerals().empty());		

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
	
	//_ASSERT(false);
	return base->Minerals().front();
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
	else if (type == BWAPI::UnitTypes::Terran_Bunker)
	{
		if (!ArmyModule::Instance()->Bunker())
		{
			return (BWAPI::TilePosition)ArmyModule::Instance()->DefaultTask()->Position();
		}
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

const BWEM::Base* Map::NextScoutBaseStart()
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
	//use angles in steps
	int steps = (int)ceil(angle / SCOUT_ANGLE_INCREASE);
	angle = steps * (float)SCOUT_ANGLE_INCREASE;
	
	radius = Config::Units::ScoutBaseRadius();

	//get next angle on circle
	angle += (float)SCOUT_ANGLE_INCREASE;
	if (angle > 2 * PI)
		angle -= 2* (float)PI;

	//calculate point
	int newX = originX + int(radius * cos(angle));
	int newY = originY + int(radius * sin(angle));

	BWAPI::Position point = BWAPI::Position(newX, newY);
	point = GetSafeScoutPoint(point, area->Bases().front().Center());
		return point;
}

BWAPI::Position Map::DefaultTaskPosition()
{
	_ASSERT(BWEB::Map::getNaturalArea());
	_ASSERT(BWEB::Map::getMainArea());

	auto nat = BWEB::Map::getNaturalArea();
	if (!nat->Bases().empty()) //if we have natural, defend there
	{
		for (auto& base : nat->Bases())
		{
			if (((BaseInfo*)base.Ptr())->_owner == Base::Owner::PLAYER)
			{
				if (!nat->ChokePoints().empty())
				{
					for (auto& choke : nat->ChokePoints())
					{
						if(choke->GetAreas().first != BWEB::Map::getMainArea() && choke->GetAreas().second != BWEB::Map::getMainArea())
							return BWAPI::Position(choke->Center()) + (base.Center() - BWAPI::Position(choke->Center())) / 4;
					}
					
				}
			}
		}
	}
	auto main = BWEB::Map::getMainArea(); //if no natural, defend main choke
	_ASSERT(!main->Bases().empty());

	for (auto& base : main->Bases())
	{
		if (((BaseInfo*)base.Ptr())->_owner == Base::Owner::PLAYER)
		{
			if (!main->ChokePoints().empty())
			{
				return BWAPI::Position(main->ChokePoints().front()->Center()) + (base.Center() - BWAPI::Position(main->ChokePoints().front()->Center())) / 3;
			}
		}
	}

	return BWEB::Map::getMainPosition();
}

bool Map::IsStillThere(EnemyUnit & enemy)
{
	_ASSERT(enemy._type.isBuilding());
	_ASSERT(enemy._lastPos.isValid());

	for (int x = 0; x < enemy._type.tileWidth(); x++)
	{
		for (int y = 0; y < enemy._type.tileHeight(); y++)
		{
			//if we see a tile from this building and it is not there, return false
			if (BWAPI::Broodwar->isVisible(enemy._lastPos + BWAPI::TilePosition(x, y))
				&& BWAPI::Broodwar->getUnitsOnTile(enemy._lastPos + BWAPI::TilePosition(x, y),BWAPI::Filter::GetType == enemy._type).empty())
				return false;
		}
	}
	return true;
}

bool Map::IsVisible(const BWEM::Base * base)
{
	_ASSERT(base);
	_ASSERT(base->Location().isValid());

	for (int x = 0; x < BWAPI::UnitTypes::Terran_Command_Center.tileWidth(); x++)
	{
		for (int y = 0; y < BWAPI::UnitTypes::Terran_Command_Center.tileHeight(); y++)
		{
			if (BWAPI::Broodwar->isVisible(base->Location() + BWAPI::TilePosition(x, y)))
				return true;
		}
	}
	return false;
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