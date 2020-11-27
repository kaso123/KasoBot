#include "MapModule.h"
#include "Expansion.h"
#include "Config.h"
#include "WorkersModule.h"
#include "ArmyModule.h"
#include "ScoutModule.h"
#include "ProductionModule.h"
#include "BaseInfo.h"
#include "Task.h"
#include "Log.h"

#include <math.h>

#define PI 3.141593
#define TILE_SIZE 32
#define SCOUT_ANGLE_INCREASE 0.2
#define DEF_POINT_CHOKE_DISTANCE_MAIN 150
#define DEF_POINT_CHOKE_DISTANCE_NAT 100
#define MAX_RETRY 100
#define VECTOR_STEP 10
#define BASE_DIST_FAIL 30


using namespace KasoBot;

namespace {

	//check calculated point, make sure it is not blocked and is in same area as the base
	BWAPI::Position GetSafeScoutPoint(BWAPI::Position oldPoint, BWAPI::Position base)
	{
		auto area = BWEM::Map::Instance().GetArea(BWAPI::TilePosition(base));
		Log::Instance()->Assert(area,"No area found for base position!");

		auto tile = BWAPI::TilePosition(oldPoint);
		//check area
		
		int counter = 0; //safe check
		bool outOfArea = false; //set to true, when generated point was already outside of area (then we always move the point closer to base)
		while (counter < MAX_RETRY)
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
				oldPoint = BWAPI::Position((int)(oldPoint.x + VECTOR_STEP * normalized[0]), (int)(oldPoint.y + VECTOR_STEP * normalized[1]));
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
				oldPoint = BWAPI::Position((int)(oldPoint.x - VECTOR_STEP * normalized[0]), (int)(oldPoint.y - VECTOR_STEP * normalized[1]));
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
				oldPoint = BWAPI::Position((int)(oldPoint.x + VECTOR_STEP * normalized[0]), (int)(oldPoint.y + VECTOR_STEP * normalized[1]));
				tile = BWAPI::TilePosition(oldPoint);

				if (oldPoint.getDistance(base) < BASE_DIST_FAIL) //return base position if too close
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
	Log::Instance()->Assert(base,"No base when finding mineral!");
	Log::Instance()->Assert(!base->Minerals().empty(),"Base has no minerals!");

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
	
	Log::Instance()->Assert(false,"No mineral ready for worker!");
	return nullptr;
}

BWAPI::TilePosition Map::GetNextBase()
{
	Log::Instance()->Assert(BWEB::Map::getNaturalArea(),"No natural area in BWEB!");

	for (auto& base : BWEB::Map::getNaturalArea()->Bases())
	{
		if (((BaseInfo*)base.Ptr())->_owner == Base::Owner::NONE
			|| ((BaseInfo*)base.Ptr())->_owner == Base::Owner::UNKNOWN)
			return base.Location();
	}
	
	auto closestDist = DBL_MAX;
	BWAPI::TilePosition closest = BWAPI::TilePositions::Invalid;
	for (auto station : BWEB::Stations::getStations())
	{
		auto base = station.getBWEMBase();

		Log::Instance()->Assert(base,"No base in station!");

		if (!Map::CanAccess(base->GetArea())) //skip islands
			continue;
		if (((BaseInfo*)base->Ptr())->_owner == Base::Owner::PLAYER
			|| ((BaseInfo*)base->Ptr())->_owner == Base::Owner::ENEMY)
			continue;

		double dist = station.getBWEMBase()->Location().getApproxDistance(BWAPI::Broodwar->self()->getStartLocation());
		dist -= ScoutModule::Instance()->EnemyStart() ? station.getBWEMBase()->Location().getApproxDistance(ScoutModule::Instance()->EnemyStart()->Bases().front().Location()) : 0.0f;
		if (dist < closestDist)
		{
			closestDist = dist;
			closest = station.getBWEMBase()->Location();
		}
	}
	return  closest;
}

BWAPI::TilePosition Map::GetBuildPosition(BWAPI::UnitType type)
{
	if (type.isResourceDepot()) //command center
	{
		return Map::GetNextBase();
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
			auto pos = (BWAPI::TilePosition)ArmyModule::Instance()->DefaultTask()->Position();
			if (BWEB::Map::isPlaceable(BWAPI::UnitTypes::Terran_Bunker, pos))
				return pos;
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

	Log::Instance()->Assert(false,"Didn't find unfinished building on tile!");
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
	Log::Instance()->Assert(BWEB::Map::getNaturalArea(), "No nat area in BWEB!");
	Log::Instance()->Assert(BWEB::Map::getMainArea(), "No main area in BWEB!");
	
	//if getting worker rushed defend around cc
	if (ScoutModule::Instance()->EnemyWorkerRush() && ArmyModule::Instance()->GetArmySupply(false) < 16) //TODO configurable
		return BWEB::Map::getMainPosition();

	auto nat = BWEB::Map::getNaturalArea();
	if (!nat->Bases().empty()) //if we have natural, defend there
	{
		for (auto& base : nat->Bases())
		{
			if (((BaseInfo*)base.Ptr())->_owner == Base::Owner::PLAYER || ProductionModule::Instance()->IsBaseInProgress(&base))
			{
				if (!nat->ChokePoints().empty())
				{
					for (auto& choke : nat->ChokePoints())
					{
						if (choke->GetAreas().first != BWEB::Map::getMainArea() && choke->GetAreas().second != BWEB::Map::getMainArea())
						{
							//Get point where bunker can be built and is close to choke
							auto pos1 = BWAPI::Position(choke->Pos(BWEM::ChokePoint::node::end1));
							auto pos2 = BWAPI::Position(choke->Pos(BWEM::ChokePoint::node::end2));
							
							if (pos1 == pos2) //single point choke, can't get vector
							{
								return pos1; //return point on choke
							}

							BWAPI::Position chokeVectorInt = pos1 - pos2;
							
							//normal vector for choke
							std::pair<float, float> normal = { (float)chokeVectorInt.y, -(float)chokeVectorInt.x };

							//normalize normal vector (sqrt(x^2+y^2))
							float length = sqrt(normal.first * normal.first + normal.second * normal.second);
							normal.first = normal.first / length;
							normal.second = normal.second / length;

							//position outside of choke
							BWAPI::Position startPoint = BWAPI::Position(choke->Pos(BWEM::ChokePoint::node::middle)) 
								+ BWAPI::Position(int(normal.first * DEF_POINT_CHOKE_DISTANCE_NAT),int(normal.second * DEF_POINT_CHOKE_DISTANCE_NAT));

							//reverse if point is on other side of choke
							if(BWEM::Map::Instance().GetArea(BWAPI::TilePosition(startPoint)) != BWEB::Map::getNaturalArea())
								startPoint = BWAPI::Position(choke->Pos(BWEM::ChokePoint::node::middle))
									+ BWAPI::Position(int(-normal.first * DEF_POINT_CHOKE_DISTANCE_NAT), int(-normal.second * DEF_POINT_CHOKE_DISTANCE_NAT));

							//vector towards base
							BWAPI::Position vectorBase = (BWAPI::Position)BWEB::Map::getNaturalArea()->Bases().front().Center() - startPoint;
							length = sqrt((float)vectorBase.x * (float)vectorBase.x + (float)vectorBase.y * (float)vectorBase.y);
							std::pair<float, float> normVectorBase;
							normVectorBase.first = vectorBase.x / length;
							normVectorBase.second = vectorBase.y / length;
							
							while (1) //move closer to base until bunker is buildable
							{
								//TODO this is debug drawing
								BWAPI::Broodwar->registerEvent([startPoint](BWAPI::Game*) { BWAPI::Broodwar->drawBoxMap(startPoint,startPoint + BWAPI::Position(96,64),BWAPI::Colors::Orange,false); },   // action
									nullptr,    // condition
									500);  // frames to run

								if (BWEB::Map::isPlaceable(BWAPI::UnitTypes::Terran_Bunker, BWAPI::TilePosition(startPoint))
									|| BWEB::Map::isUsed(BWAPI::TilePosition(startPoint)) == BWAPI::UnitTypes::Terran_Bunker)
									return startPoint;

								startPoint = startPoint + BWAPI::Position(int(normVectorBase.first * 10), int(normVectorBase.second * 10));

								//if too close to base, return choke position
								if (startPoint.getDistance((BWAPI::Position)BWEB::Map::getNaturalArea()->Bases().front().Center()) < 100)
									return BWAPI::Position(choke->Pos(BWEM::ChokePoint::node::middle));
							}
						}	
					}
					
				}
			}
		}
	}
	auto main = BWEB::Map::getMainArea(); //if no natural, defend main choke
	Log::Instance()->Assert(!main->Bases().empty(), "Main area has no bases!");

	for (auto& base : main->Bases())
	{
		if (((BaseInfo*)base.Ptr())->_owner == Base::Owner::PLAYER)
		{
			//TODO this code is doubled for main and natural

			if (!main->ChokePoints().empty())
			{
				auto choke = main->ChokePoints().front();

				//Get point where bunker can be built and is close to choke
				auto pos1 = BWAPI::Position(choke->Pos(BWEM::ChokePoint::node::end1));
				auto pos2 = BWAPI::Position(choke->Pos(BWEM::ChokePoint::node::end2));

				if (pos1 == pos2) //single point choke, can't get vector
				{
					return pos1; //return point on choke
				}

				BWAPI::Position chokeVectorInt = pos1 - pos2;

				//normal vector for choke
				std::pair<float, float> normal = { (float)chokeVectorInt.y, -(float)chokeVectorInt.x };

				//normalize normal vector (sqrt(x^2+y^2))
				float length = sqrt(normal.first * normal.first + normal.second * normal.second);
				normal.first = normal.first / length;
				normal.second = normal.second / length;

				//position outside of choke
				BWAPI::Position startPoint = BWAPI::Position(choke->Pos(BWEM::ChokePoint::node::middle))
					+ BWAPI::Position(int(normal.first * DEF_POINT_CHOKE_DISTANCE_MAIN), int(normal.second * DEF_POINT_CHOKE_DISTANCE_MAIN));

				//reverse if point is on other side of choke
				if (BWEM::Map::Instance().GetArea(BWAPI::TilePosition(startPoint)) != BWEB::Map::getMainArea())
					startPoint = BWAPI::Position(choke->Pos(BWEM::ChokePoint::node::middle))
					+ BWAPI::Position(int(-normal.first * DEF_POINT_CHOKE_DISTANCE_MAIN), int(-normal.second * DEF_POINT_CHOKE_DISTANCE_MAIN));

				//vector towards base
				BWAPI::Position vectorBase = (BWAPI::Position)BWEB::Map::getMainArea()->Bases().front().Center() - startPoint;
				length = sqrt((float)vectorBase.x * (float)vectorBase.x + (float)vectorBase.y * (float)vectorBase.y);
				std::pair<float, float> normVectorBase;
				normVectorBase.first = vectorBase.x / length;
				normVectorBase.second = vectorBase.y / length;
				

				while (1) //move closer to base until bunker is buildable
				{
					//TODO this is debug drawing
					BWAPI::Broodwar->registerEvent([startPoint](BWAPI::Game*) { BWAPI::Broodwar->drawBoxMap(startPoint, startPoint + BWAPI::Position(96, 64), BWAPI::Colors::Orange, false); },   // action
						nullptr,    // condition
						500);  // frames to run

					if (BWEB::Map::isPlaceable(BWAPI::UnitTypes::Terran_Bunker, BWAPI::TilePosition(startPoint))
						|| BWEB::Map::isUsed(BWAPI::TilePosition(startPoint)) == BWAPI::UnitTypes::Terran_Bunker)
						return startPoint;

					startPoint = startPoint + BWAPI::Position(int(normVectorBase.first * 10), int(normVectorBase.second * 10));

					//if too close to base, return choke position
					if (startPoint.getDistance((BWAPI::Position)BWEB::Map::getMainArea()->Bases().front().Center()) < 100)
						return BWAPI::Position(choke->Pos(BWEM::ChokePoint::node::middle));
				}
			}
		}
	}

	return BWEB::Map::getMainPosition();
}

bool Map::IsStillThere(EnemyUnit & enemy)
{
	Log::Instance()->Assert(enemy._type.isBuilding(), "Wrong enemy type in IsStillThere!");
	Log::Instance()->Assert(enemy._lastPos.isValid(), "invalid position in IsStillThere!");

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
	Log::Instance()->Assert(base,"No base in IsVisible!");
	Log::Instance()->Assert(base->Location().isValid(),"Base location is invalid!");

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

bool Map::CanAccess(const BWEM::Area * area)
{
	if (!area)
		return false;

	Log::Instance()->Assert(BWEB::Map::getMainArea(),"No main area in BWEB!");

	if (area->AccessibleFrom(BWEB::Map::getMainArea()))
		return true;
	return false;
}

BWAPI::Position Map::GetHarassContactPoint(const BWEM::Base * base)
{
	//return position of first mineral in base (base itself when no minerals)
	if (base->Minerals().empty())
		return base->Center();

	if (!base->Minerals().front()->Unit())
		return base->Center();

	if (!base->Minerals().front()->Unit()->getInitialPosition().isValid())
		return base->Center();

	return base->Minerals().front()->Unit()->getInitialPosition();
}

BWAPI::Position Map::GetHarassCheckpoint(const BWEM::Base * base)
{
	//closer to us
	if (!ScoutModule::Instance()->EnemyNatural() || base->GetArea() == ScoutModule::Instance()->EnemyNatural()
		|| base->Center().getApproxDistance(BWEB::Map::getMainPosition()) < base->Center().getApproxDistance(ScoutModule::Instance()->EnemyNatural()->Bases().front().Center()))
	{
		//vector
		BWAPI::Position vectorInt = base->Center() - BWEB::Map::getMainPosition();

		//normal vector
		std::pair<float, float> normal = { (float)vectorInt.y, -(float)vectorInt.x };

		//normalize normal vector (sqrt(x^2+y^2))
		float length = sqrt(normal.first * normal.first + normal.second * normal.second);
		normal.first = normal.first / length;
		normal.second = normal.second / length;

		BWAPI::Position left = base->Center();
		BWAPI::Position right = base->Center();
		//move points until hitting end of map
		while (true)
		{
			//left
			left = left + BWAPI::Position(int(normal.first * 10), int(normal.second * 10));
			if (!left.isValid())
			{
				return ClipIntoMap(left);
			}

			//right
			right = right + BWAPI::Position(int(normal.first * -10), int(normal.second * -10));
			if (!right.isValid())
			{
				return ClipIntoMap(right);
			}
		}
	}
	else //closer to enemy
	{
		//line from enemy main to base
		BWAPI::Position vectorInt = base->Center() - ScoutModule::Instance()->EnemyNatural()->Bases().front().Center();

		std::pair<float, float> vector = { (float)vectorInt.x, (float)vectorInt.y };

		//normalize normal vector (sqrt(x^2+y^2))
		float length = sqrt(vector.first * vector.first + vector.second * vector.second);
		vector.first = vector.first / length;
		vector.second = vector.second / length;

		BWAPI::Position point = base->Center();
		//ending is the point
		while (true)
		{
			point = point + BWAPI::Position(int(vector.first * 30), int(vector.second * 30));

			if (!point.isValid())
				return ClipIntoMap(point);
		}
	}
}

BWAPI::Position Map::ClipIntoMap(BWAPI::Position pos)
{
	return BWAPI::Position(
		std::clamp(pos.x, 1, BWAPI::Broodwar->mapWidth() * 32 - 1),
		std::clamp(pos.y, 1, BWAPI::Broodwar->mapHeight() * 32 - 1)
	);
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