#include "Unit.h"
#include "BehaviourWorker.h"
#include "BehaviourMedic.h"
#include "BehaviourVessel.h"
#include "ArmyModule.h"
#include "MapModule.h"
#include "Army.h"
#include "Task.h"

using namespace KasoBot;

namespace {
	void SetProperBehaviourType(std::unique_ptr<KasoBot::Behaviour>& outBehaviour, BWAPI::UnitType type)
	{
		if (type.isWorker())
		{
			outBehaviour = std::make_unique<BehaviourWorker>();
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

	_ASSERT(_behaviour);
	_ASSERT(army && army->Task());
	
	//TODO do priority things (in close combat)

	//move to center of army
	if (_pointer->getPosition().getDistance(army->BoundingBox()._center) > Config::Units::ArmyRange() * TILE_SIZE) //different value for our armies
	{
		_behaviour->MoveToArmyCenter(*this, army->BoundingBox()._center);
		return;
	}

	//TODO do task things
	if (army->Task()->Type() == Tasks::Type::ATTACK)
	{
		_behaviour->AttackArea(*this, army);
	}
	else if (army->Task()->Type() == Tasks::Type::DEFEND)
	{
		_behaviour->DefendArmy(*this, army);
	}
}

void Unit::Scout()
{
	if (_playerControl)
		return;

	_ASSERT(_behaviour);
	_behaviour->Scout(*this);
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
	
	if (_pointer->move(BWEM::Map::Instance().RandomPosition()))
		_clearTileLock = BWAPI::Broodwar->getFrameCount();
}
