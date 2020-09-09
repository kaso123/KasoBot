#include "Unit.h"
#include "BehaviourWorker.h"
#include "ArmyModule.h"

using namespace KasoBot;

Unit::Unit(BWAPI::Unit unit)
	:_pointer(unit), _playerControl(false), _playerControlFrame(0)
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

	_ASSERT(_behaviour);
	//TODO call function from behaviour
}

void Unit::ChangeDebugControl()
{
	if (_playerControlFrame + Config::Units::OrderDelay() > BWAPI::Broodwar->getFrameCount())
		return;

	_playerControl = !_playerControl;
	_playerControlFrame = BWAPI::Broodwar->getFrameCount();
}
