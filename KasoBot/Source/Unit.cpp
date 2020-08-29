#include "Unit.h"
#include "BehaviourWorker.h"

using namespace KasoBot;

Unit::Unit(BWAPI::Unit unit)
	:_pointer(unit), _playerControl(false)
{
	if (unit->getType().isWorker())
		_behaviour = std::make_unique<BehaviourWorker>();
	else _behaviour = std::make_unique<Behaviour>();
}

Unit::~Unit()
{
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
	_playerControl = !_playerControl;
}
