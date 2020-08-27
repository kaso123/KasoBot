#include "Unit.h"
#include "BehaviourWorker.h"

using namespace KasoBot;

Unit::Unit(BWAPI::Unit unit)
	:_pointer(unit)
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
	//TODO call function from behaviour
}
