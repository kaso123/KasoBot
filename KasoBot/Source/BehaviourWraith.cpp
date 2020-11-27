#include "BehaviourWraith.h"
#include "Army.h"
#include "Unit.h"
#include "Task.h"
#include "MapModule.h"
#include "ScoutModule.h"
#include "EnemyArmy.h"
#include "Log.h"
#include "BaseInfo.h"
#include "MapModule.h"
#include "Config.h"

#define HARASS_START_DIST 100

using namespace KasoBot;


bool BehaviourWraith::HandleCloak(KasoBot::Unit& unit)
{
	if(!BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Cloaking_Field)) //cloak is not available
		return false;

	//if close to enemy army with anti air and we have energy -> cloak
	for (auto& army : ScoutModule::Instance()->GetArmies())
	{
		if (army->AntiAirCount() <= 0)
			continue;

		if (unit.GetPointer()->getDistance(army->BoundingBox()._center) < Config::Units::ArmyRange() * 32 + Config::Units::EnemyArmyRange() * 32)
		{
			if (unit.GetPointer()->isCloaked() || unit.GetPointer()->getEnergy() < 25)
				return false; //nothing has been issued

			unit.GetPointer()->cloak();
			return true; //issued cloak command
		}
	}
	
	//no anti air enemy close
	if (!unit.GetPointer()->isCloaked())
		return false;

	unit.GetPointer()->decloak();
	return true;
}

bool BehaviourWraith::HandleDisengage(KasoBot::Unit& unit)
{
	for (auto& army : ScoutModule::Instance()->GetArmies())
	{
		if (army->AntiAirCount() <= 0) //don't disengage when no anti air
			continue;

		//TODO get better distance calculations ->distance to bounding box, not center
		if (unit.GetPointer()->getDistance(army->BoundingBox()._center) < Config::Units::ArmyRange() * 32 + Config::Units::EnemyArmyRange() * 32)
		{
			if (unit.GetPointer()->isDetected())
				Disengage(unit, *army);
				return true;
		}
	}
	return false;
}

void BehaviourWraith::Disengage(KasoBot::Unit& unit, EnemyArmy & army)
{
	auto vector = unit.GetPointer()->getPosition() - army.BoundingBox()._center;
	auto point = unit.GetPointer()->getPosition() + vector;

	Move(unit.GetPointer(), Map::ClipIntoMap(point));
}

BehaviourWraith::BehaviourWraith()
{
}

BehaviourWraith::~BehaviourWraith()
{
}

void BehaviourWraith::HarassArea(KasoBot::Unit & unit, Army * army)
{
	Log::Instance()->Assert(army, "No army in behaviour!");
	auto area = army->Task()->Area();
	Log::Instance()->Assert(!area->Bases().empty(), "No bases in area in harass task!");

	if (HandleCloak(unit))
		return;

	if (HandleDisengage(unit))
		return;

	bool cp = army->Task()->IsCheckpointDone();

	for (auto & base : area->Bases())
	{
		if (((BaseInfo*)base.Ptr())->_owner == Base::Owner::ENEMY
			|| ((BaseInfo*)base.Ptr())->_owner == Base::Owner::UNKNOWN)
		{
			if (cp) //move to harass base
			{
				BWAPI::Position pos = Map::GetHarassContactPoint(&base);
				Log::Instance()->Assert(pos.isValid(), "Invalid contact point for base harassment!");

				//attack move if close
				if (unit.GetPointer()->getDistance(pos) < HARASS_START_DIST)
				{
					AttackMove(unit.GetPointer(), pos, true);
					return;
				}
				else //move if not in position
				{
					Move(unit.GetPointer(), pos);
					return;
				}				
			}
			else //move to cp
			{
				BWAPI::Position pos = Map::GetHarassCheckpoint(&base);
				Log::Instance()->Assert(pos.isValid(), "Invalid checkpoint for base harassment!");

				if (unit.GetPointer()->getDistance(pos) < HARASS_START_DIST)
					army->Task()->SetCheckpointDone();
				Move(unit.GetPointer(), pos);
				return;
			}
			return;
		}
	}
}

void BehaviourWraith::SupportArmy(KasoBot::Unit & unit, Army * army)
{
	if (HandleCloak(unit))
		return;

	Behaviour::SupportArmy(unit, army);
}
