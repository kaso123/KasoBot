#include "Behaviour.h"
#include "Unit.h"

using namespace KasoBot;

void Behaviour::Move(BWAPI::Unit unit, BWAPI::Position position)
{
	if (unit->getOrder() == BWAPI::Orders::Move && unit->getOrderTargetPosition() == position)
		return;

	unit->move(position);
}

Behaviour::Behaviour()
{
}

Behaviour::~Behaviour()
{
}
