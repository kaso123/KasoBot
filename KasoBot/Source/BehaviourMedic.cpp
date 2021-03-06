#include "BehaviourMedic.h"
#include "Army.h"
#include "MapModule.h"
#include "Log.h"

using namespace KasoBot;

void BehaviourMedic::AttackMove(BWAPI::Unit unit, BWAPI::Position position, bool)
{
	if (unit->getOrder() == BWAPI::Orders::HealMove && unit->getOrderTargetPosition().getDistance(position) < 50)
		return;
	if (unit->getOrder() == BWAPI::Orders::AttackMove && unit->getOrderTargetPosition().getDistance(position) < 50)
		return;
	if (unit->getOrder() == BWAPI::Orders::MedicHeal)
		return;
	if (unit->getOrder() == BWAPI::Orders::Medic && unit->getDistance(position) < 50)
		return;

	unit->attack(position);
}

BehaviourMedic::BehaviourMedic()
{
}

BehaviourMedic::~BehaviourMedic()
{
}

void BehaviourMedic::AttackArea(KasoBot::Unit & unit, Army * army)
{
	Log::Instance()->Assert(army, "No army in behaviour!");
	MoveToArmyCenter(unit, army->BoundingBox()._center);
}

void BehaviourMedic::DefendArmy(KasoBot::Unit & unit, Army * army)
{
	Log::Instance()->Assert(army,"No army in behaviour!");
	MoveToArmyCenter(unit, army->BoundingBox()._center);
}
