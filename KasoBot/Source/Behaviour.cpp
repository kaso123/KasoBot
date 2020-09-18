#include "Behaviour.h"
#include "Unit.h"
#include "MapModule.h"

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

void Behaviour::Scout(KasoBot::Unit & unit)
{
	Move(unit.GetPointer(), BWEM::Map::Instance().Center());
}
