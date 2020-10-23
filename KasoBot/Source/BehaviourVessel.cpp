#include "BehaviourVessel.h"
#include "Unit.h"
#include "MapModule.h"
#include "Army.h"
#include "Log.h"

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
	Log::Instance()->Assert(army, "Army is nullptr in behaviour!");
	MoveToArmyCenter(unit, army->BoundingBox()._center);
}

void BehaviourVessel::DefendArmy(KasoBot::Unit & unit, Army* army)
{
	Log::Instance()->Assert(army,"Army is nullptr in behaviour!");
	MoveToArmyCenter(unit, army->BoundingBox()._center);
}
