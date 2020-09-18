#include "Unit.h"
#include "BehaviourWorker.h"
#include "ArmyModule.h"
#include "MapModule.h"

using namespace KasoBot;

Unit::Unit(BWAPI::Unit unit)
	:_pointer(unit), _playerControl(false), _playerControlFrame(0), _lock(false), _clearTileLock(-1), _role(Units::Role::IDLE)
{
	if (unit->getType().isWorker())
		_behaviour = std::make_unique<BehaviourWorker>();
	else _behaviour = std::make_unique<Behaviour>();
}

Unit::~Unit()
{
	if (_pointer->getType().isBuilding() || _pointer->getType().isWorker())
		return;

	ArmyModule::Instance()->SoldierKilled(this);
}

void Unit::Fight()
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
	//TODO call function from behaviour
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
