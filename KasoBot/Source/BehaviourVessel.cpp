#include "BehaviourVessel.h"
#include "Unit.h"
#include "MapModule.h"
#include "Army.h"

using namespace KasoBot;

void BehaviourVessel::AttackMove(BWAPI::Unit unit, BWAPI::Position position)
{
	Move(unit, position);
}

BehaviourVessel::BehaviourVessel()
{
}

BehaviourVessel::~BehaviourVessel()
{
}

void BehaviourVessel::AttackArea(KasoBot::Unit & unit, Army* army)
{
	_ASSERT(army);
	MoveToArmyCenter(unit, army->BoundingBox()._center);
}
