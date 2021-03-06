#include "Unit.h"
#include "BehaviourWorker.h"
#include "BehaviourMedic.h"
#include "BehaviourVessel.h"
#include "BehaviourMarine.h"
#include "BehaviourWraith.h"
#include "ArmyModule.h"
#include "MapModule.h"
#include "ScoutModule.h"
#include "Army.h"
#include "Task.h"
#include "Log.h"

using namespace KasoBot;

namespace {
	void SetProperBehaviourType(std::unique_ptr<KasoBot::Behaviour>& outBehaviour, BWAPI::UnitType type)
	{
		if (type.isWorker())
		{
			outBehaviour = std::make_unique<BehaviourWorker>();
			return;
		}
		if (type == BWAPI::UnitTypes::Terran_Marine)
		{
			outBehaviour = std::make_unique<BehaviourMarine>();
			return;
		}
		if (type == BWAPI::UnitTypes::Terran_Medic)
		{
			outBehaviour = std::make_unique<BehaviourMedic>();
			return;
		}
		if (type == BWAPI::UnitTypes::Terran_Science_Vessel)
		{
			outBehaviour = std::make_unique<BehaviourVessel>();
			return;
		}
		if (type == BWAPI::UnitTypes::Terran_Wraith)
		{
			outBehaviour = std::make_unique<BehaviourWraith>();
			return;
		}
		
		outBehaviour = std::make_unique<Behaviour>();
	}
}

Unit::Unit(BWAPI::Unit unit)
	:_pointer(unit), _playerControl(false), _playerControlFrame(0), _lock(false), _clearTileLock(-1), _role(Units::Role::IDLE)
{
	SetProperBehaviourType(_behaviour,unit->getType());
}

Unit::~Unit()
{
	if (_pointer->getType().isBuilding() || _pointer->getType().isWorker())
		return;

	ArmyModule::Instance()->SoldierKilled(this);
}

void Unit::Fight(Army* army)
{
	if (_playerControl)
		return;

	//if this unit is moving away to clear space for building
	//wait for some time before doing anything else
	if (_clearTileLock > 0)
	{
		if ((_clearTileLock + Config::Units::ClearTileLock()) < BWAPI::Broodwar->getFrameCount())
			_clearTileLock = -1;

		return;
	}

	Log::Instance()->Assert(_behaviour.get(),"Behaviour missing!");
	Log::Instance()->Assert(army && army->Task(),"Army or task missing in Unit::Fight!");
	
	//TODO do priority things (in close combat)

	//move to center of army
	if ((army->Task()->Type() == Tasks::Type::ATTACK || army->Task()->Type() == Tasks::Type::HARASS) && BWAPI::Broodwar->isWalkable((BWAPI::WalkPosition)army->BoundingBox()._center)
		&& _pointer->getPosition().getDistance(army->BoundingBox()._center) > Config::Units::ArmyRange() * TILE_SIZE) //different value for our armies
	{
		_behaviour->MoveToArmyCenter(*this, army->BoundingBox()._center);
		return;
	}

	//do task things
	if (army->Task()->Type() == Tasks::Type::ATTACK)
	{
		_behaviour->AttackArea(*this, army);
	}
	else if (army->Task()->Type() == Tasks::Type::DEFEND)
	{
		_behaviour->DefendArmy(*this, army);
	}
	else if (army->Task()->Type() == Tasks::Type::SCOUT)
	{
		_behaviour->ScoutArea(*this, army);
	}
	else if (army->Task()->Type() == Tasks::Type::HOLD)
	{
		_behaviour->HoldPosition(*this, army);
	}
	else if (army->Task()->Type() == Tasks::Type::FINISH)
	{
		_behaviour->FinishEnemy(*this, army);
	}
	else if (army->Task()->Type() == Tasks::Type::SUPPORT)
	{
		_behaviour->SupportArmy(*this, army);
	}
	else if (army->Task()->Type() == Tasks::Type::HARASS)
	{
		_behaviour->HarassArea(*this, army);
	}
}

void Unit::Scout()
{
	if (_playerControl)
		return;

	Log::Instance()->Assert(_behaviour.get(),"Behaviour missing in Scout!");
	_behaviour->Scout(*this);
}

void Unit::ScoutRush()
{
	if (_playerControl)
		return;

	if (!ScoutModule::Instance()->ShouldWorkerScoutRush())
	{
		_role = Units::Role::IDLE;
		return;
	}

	Log::Instance()->Assert(_behaviour.get(), "Behaviour missing in ScoutRush!");
	_behaviour->ScoutRush(*this);
}

void Unit::ChangeDebugControl()
{
	if (_playerControlFrame + Config::Units::OrderDelay() > BWAPI::Broodwar->getFrameCount())
		return;

	_playerControl = !_playerControl;
	_playerControlFrame = BWAPI::Broodwar->getFrameCount();
}

void Unit::ClearTile()
{
	if (_clearTileLock > 0)
		return;
	
	if (_pointer->attack(BWEM::Map::Instance().RandomPosition()))
		_clearTileLock = BWAPI::Broodwar->getFrameCount();
}
