#include "Behaviour.h"
#include "Unit.h"
#include "MapModule.h"
#include "ScoutModule.h"
#include "BaseInfo.h"
#include "Config.h"
#include "Army.h"
#include "EnemyArmy.h"
#include "Task.h"

using namespace KasoBot;

void Behaviour::Move(BWAPI::Unit unit, BWAPI::Position position)
{
	if (unit->getOrder() == BWAPI::Orders::Move && unit->getOrderTargetPosition() == position)
		return;

	unit->move(position);
}

void Behaviour::AttackMove(BWAPI::Unit unit, BWAPI::Position position)
{
	if (unit->getOrder() == BWAPI::Orders::AttackMove && unit->getOrderTargetPosition().getDistance(position) < 50) //TODO set this value as configurable
		return;
	if (unit->getOrder() == BWAPI::Orders::AttackUnit)
		return;

	unit->attack(position);
}

void Behaviour::HoldPosition(BWAPI::Unit unit)
{
	if (unit->getOrder() == BWAPI::Orders::HoldPosition)
		return;

	unit->holdPosition();
}

Behaviour::Behaviour()
{
}

Behaviour::~Behaviour()
{
}

void Behaviour::AttackArea(KasoBot::Unit & unit, Army* army)
{
	_ASSERT(army);
	auto area = army->Task()->Area();
	_ASSERT(!area->Bases().empty());
	
	for (auto & base : area->Bases())
	{
		if (((BaseInfo*)base.Ptr())->_owner == Base::Owner::ENEMY
			|| ((BaseInfo*)base.Ptr())->_owner == Base::Owner::UNKNOWN)
		{
			AttackMove(unit.GetPointer(),base.Center());
		}
	}
}

void Behaviour::ScoutArea(KasoBot::Unit & unit, Army * army)
{
	_ASSERT(army);
	auto area = army->Task()->Area();
	_ASSERT(!area->Bases().empty());

	for (auto & base : area->Bases())
	{
		if (((BaseInfo*)base.Ptr())->_owner == Base::Owner::UNKNOWN)
		{
			Move(unit.GetPointer(), base.Center());
		}
	}
}

void Behaviour::DefendArmy(KasoBot::Unit& unit, Army* army)
{
	_ASSERT(army);
	auto enemyArmy = army->Task()->EnemyArmy();
	_ASSERT(enemyArmy);

	AttackMove(unit.GetPointer(), enemyArmy->BoundingBox()._center);
}

void Behaviour::HoldPosition(KasoBot::Unit & unit, Army * army)
{
	_ASSERT(army);
	auto pos = army->Task()->Position();
	_ASSERT(pos.isValid());

	if (unit.GetPointer()->getDistance(pos) > 40) //TODO make configurable
		AttackMove(unit.GetPointer(), pos);
	else
		HoldPosition(unit.GetPointer());
}

void Behaviour::MoveToArmyCenter(KasoBot::Unit & unit, BWAPI::Position position)
{
	_ASSERT(position.isValid());

	AttackMove(unit.GetPointer(), position);
}

void Behaviour::Scout(KasoBot::Unit & unit)
{
	if (ScoutModule::Instance()->EnemyStart())
	{
		BWAPI::Position pos = BWAPI::Positions::Invalid;

		//scout enemy natural
		if (ScoutModule::Instance()->EnemyNatural()
			&& ((BaseInfo*)ScoutModule::Instance()->EnemyNatural()->Bases().front().Ptr())->_owner == Base::Owner::UNKNOWN)
		{
			pos = ScoutModule::Instance()->EnemyNatural()->Bases().front().Center();
		}
		else //scout enemy tech inside base
		{
			pos = Map::NextScoutPosition(ScoutModule::Instance()->EnemyStart(), unit.GetPointer()->getPosition());
		}

		if (!pos.isValid())
			return;

		//TODO this is debug drawing
		BWAPI::Broodwar->registerEvent([pos](BWAPI::Game*) { BWAPI::Broodwar->drawCircleMap(pos,5,BWAPI::Colors::Blue,true); },   // action
			nullptr,    // condition
			BWAPI::Broodwar->getLatencyFrames());  // frames to run

		Move(unit.GetPointer(), pos);

		return;
	}

	auto base = Map::NextScoutBaseStart();
	if (base)
	{
		//if close to base and didn't see an enemy building set it as free
		if (unit.GetPointer()->getDistance(base->Center()) <= Config::Workers::BuildStartDistance())
		{
			((BaseInfo*)base->Ptr())->_owner = Base::Owner::NONE;
			return;
		}

		//move to base
		Move(unit.GetPointer(), base->Center());
	}
}
