#include "Task.h"
#include "MapModule.h"
#include "EnemyArmy.h"
#include "ScoutModule.h"
#include "StrategyModule.h"
#include "OwnStrategy.h"
#include "Army.h"
#include "BaseInfo.h"
#include "Config.h"
#include "Log.h"

using namespace KasoBot;


Task::Task(Tasks::Type type)
	:_type(type), _inProgress(false), _finished(false)
{
}

Task::~Task()
{
}

AttackAreaTask::AttackAreaTask(const BWEM::Area * area)
	: Task(Tasks::Type::ATTACK), _area(area)
{
}

bool AttackAreaTask::IsArmySuitable(Army & army)
{
	if (army.IsAir())
		return false;

	if (army.GetSupply() < StrategyModule::Instance()->GetActiveStrat()->MinArmySupply())
		return  false;
	return true;
}

bool AttackAreaTask::IsFinished()
{
	if (_finished)
		return true;

	Log::Instance()->Assert(_area,"No area in attack task!");

	if (_area->Bases().empty())
	{
		_finished = true;
		return true;
	}

	for (auto& base : _area->Bases()) //if all bases in area are ours or empty, finish task
	{
		if (((BaseInfo*)base.Ptr())->_owner == Base::Owner::ENEMY
			|| ((BaseInfo*)base.Ptr())->_owner == Base::Owner::UNKNOWN)
			return false;
	}
	_finished = true;
	return true;
}

HoldPositionTask::HoldPositionTask(BWAPI::Position pos)
	: Task(Tasks::Type::HOLD), _pos(pos)
{
}

DefendArmyTask::DefendArmyTask(KasoBot::EnemyArmy * army)
	: Task(Tasks::Type::DEFEND), _army(army)
{
}

bool DefendArmyTask::IsArmySuitable(Army& army)
{
	return !army.IsAir() && !(_army->IsOnlyFlying() && army.AntiAirCount() <= 0);
}

bool DefendArmyTask::IsFinished()
{
	return !_army->IsThreat();
}

ScoutAreaTask::ScoutAreaTask(const BWEM::Area * area)
	: Task(Tasks::Type::SCOUT), _area(area)
{
}

bool ScoutAreaTask::IsArmySuitable(Army & army)
{
	if ((army.Units().size() == 1 && army.GetScoutSoldier()) || BWAPI::Broodwar->self()->supplyUsed() > 380) //TODO make configurable
		return true;

	return false;
}

bool ScoutAreaTask::IsFinished()
{
	Log::Instance()->Assert(_area,"No area in scout task!");

	if (_area->Bases().empty())
		return true;

	for (auto& base : _area->Bases()) //if there are no unknown bases, finish task
	{
		if (((BaseInfo*)base.Ptr())->_owner == Base::Owner::UNKNOWN)
			return false;
	}
	return true;
}

FinishEnemyTask::FinishEnemyTask()
	: Task(Tasks::Type::FINISH), _nextPos(BWAPI::TilePositions::Invalid)
{
}

bool FinishEnemyTask::IsArmySuitable(Army& army)
{
	return !army.IsAir();
}

BWAPI::TilePosition FinishEnemyTask::Next()
{
	if (!_nextPos.isValid() || BWAPI::Broodwar->isVisible(_nextPos))
	{
		_nextPos = BWAPI::TilePositions::Invalid;
	}

	while (!_nextPos.isValid())
	{
		_nextPos = BWAPI::TilePosition(BWEM::Map::Instance().RandomPosition());

		if (!_nextPos.isValid() || BWAPI::Broodwar->isVisible(_nextPos)
			|| !BWEM::Map::Instance().GetNearestArea(_nextPos)->AccessibleFrom(BWEB::Map::getMainArea()))
			_nextPos = BWAPI::TilePositions::Invalid;
	}

	Log::Instance()->Assert(_nextPos.isValid(), "invalid random position for finish task!");
	return _nextPos;
}

SupportArmyTask::SupportArmyTask(Army* army)
	: Task(Tasks::Type::SUPPORT), _army(army)
{
}

bool SupportArmyTask::IsArmySuitable(Army& army)
{
	return army.IsAir();
}

bool SupportArmyTask::IsFinished()
{
	return _army->GetSupply() <= 0;
}

HarassAreaTask::HarassAreaTask(const BWEM::Area * area)
	: Task(Tasks::Type::HARASS), _area(area), _cp(false)
{
}

void HarassAreaTask::Start()
{
	_cp = false;
	Task::Start();
}

bool HarassAreaTask::IsArmySuitable(Army & army)
{
	if (army.CanHarass())
		return true;

	return false;
}

bool HarassAreaTask::IsFinished()
{
	if (_finished)
		return true;

	Log::Instance()->Assert(_area, "No area in harass task!");

	if (_area->Bases().empty())
	{
		_finished = true;
		return true;
	}

	for (auto& base : _area->Bases()) //if all bases in area are ours or empty, finish task
	{
		if (((BaseInfo*)base.Ptr())->_owner == Base::Owner::ENEMY
			|| ((BaseInfo*)base.Ptr())->_owner == Base::Owner::UNKNOWN)
			return false;
	}
	_finished = true;
	return true;
}
