#include "BehaviourMarine.h"
#include "ArmyModule.h"
#include "Army.h"
#include "EnemyArmy.h"
#include "Unit.h"
#include "Task.h"
#include "MapModule.h"
#include "Log.h"

using namespace KasoBot;

void BehaviourMarine::GoInBunker(BWAPI::Unit unit, BWAPI::Unit bunker)
{
	if (unit->getOrder() == BWAPI::Orders::EnterTransport && unit->getOrderTarget() == bunker)
		return;

	unit->rightClick(bunker);
}

void BehaviourMarine::LeaveBunker(BWAPI::Unit unit, BWAPI::Unit bunker)
{
	if (!unit->isLoaded())
		return;
	bunker->unload(unit);
}

BehaviourMarine::BehaviourMarine()
{
}

BehaviourMarine::~BehaviourMarine()
{
}

void BehaviourMarine::DefendArmy(KasoBot::Unit & unit, Army * army)
{
	if (!ArmyModule::Instance()->Bunker())
	{
		Behaviour::DefendArmy(unit, army);
		return;
	}

	if (army->Task()->EnemyArmy()->IsCannonRush())
	{
		if (unit.GetPointer()->isLoaded() && ArmyModule::Instance()->Bunker())
		{
			LeaveBunker(unit.GetPointer(), ArmyModule::Instance()->Bunker()->GetPointer());
			return;
		}
		Behaviour::DefendArmy(unit, army);
		return;
	}

	//using bunker
	Log::Instance()->Assert(army->Task(),"Missing task from army in defend behaviour!");
	Log::Instance()->Assert(army->Task()->EnemyArmy(), "Missing enemy army from task in defend behaviour!");

	//enemy army ran past bunker
	if (ArmyModule::Instance()->Bunker()->GetPointer()->getDistance(BWEB::Map::getMainPosition()) 
		> army->Task()->EnemyArmy()->BoundingBox()._center.getDistance(BWEB::Map::getMainPosition()))
	{
		if (unit.GetPointer()->isLoaded()) //leave bunker
		{
			LeaveBunker(unit.GetPointer(), ArmyModule::Instance()->Bunker()->GetPointer());
		}
		else
		{
			Behaviour::DefendArmy(unit, army);
		}
	}
	else //enemy army is close to bunker
	{
		if (!unit.GetPointer()->isLoaded() 
			&& ArmyModule::Instance()->Bunker()->GetPointer()->getLoadedUnits().size() < (size_t)BWAPI::UnitTypes::Terran_Bunker.spaceProvided()) 
		{
			GoInBunker(unit.GetPointer(), ArmyModule::Instance()->Bunker()->GetPointer());
		}
		else
		{
			Behaviour::DefendArmy(unit, army);
		}
	}
}

void BehaviourMarine::HoldPosition(KasoBot::Unit & unit, Army * army)
{
	if (!ArmyModule::Instance()->Bunker())
	{
		Behaviour::HoldPosition(unit,army);
		return;
	}

	//using bunker

	if (ArmyModule::Instance()->Bunker()->GetPointer()->getLoadedUnits().size() >= (size_t)BWAPI::UnitTypes::Terran_Bunker.spaceProvided())
	{
		//bunker is full
		Behaviour::HoldPosition(unit, army);
		return;
	}
	if (ArmyModule::Instance()->Bunker()->GetPointer()->getDistance(army->Task()->Position()) > 200) //TODO configurable, use bunker distance
	{
		if (unit.GetPointer()->isLoaded())
		{
			LeaveBunker(unit.GetPointer(), ArmyModule::Instance()->Bunker()->GetPointer());
			return;
		}

		//task is not close to bunker
		Behaviour::HoldPosition(unit, army);
		return;
	}

	//go inside bunker
	GoInBunker(unit.GetPointer(), ArmyModule::Instance()->Bunker()->GetPointer());
}

void BehaviourMarine::AttackArea(KasoBot::Unit & unit, Army * army)
{
	if (ArmyModule::Instance()->Bunker())
	{
		if (unit.GetPointer()->isLoaded())
		{
			LeaveBunker(unit.GetPointer(), ArmyModule::Instance()->Bunker()->GetPointer());
			return;
		}
	}
	Behaviour::AttackArea(unit, army);
}

void BehaviourMarine::ScoutArea(KasoBot::Unit & unit, Army * army)
{
	if (ArmyModule::Instance()->Bunker())
	{
		if (unit.GetPointer()->isLoaded())
		{
			LeaveBunker(unit.GetPointer(), ArmyModule::Instance()->Bunker()->GetPointer());
			return;
		}
	}
	Behaviour::ScoutArea(unit, army);
}
