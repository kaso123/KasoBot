#include "Army.h"
#include "Config.h"
#include "Task.h"
#include "ArmyModule.h"
#include "WorkersModule.h"
#include "StrategyModule.h"
#include "ScoutModule.h"
#include "MapModule.h"
#include "OwnStrategy.h"
#include "EnemyArmy.h"
#include "Worker.h"
#include "Log.h"
#include "BaseInfo.h"

using namespace KasoBot;

void Army::CalculateCenter()
{
	int minX = INT_MAX;
	int minY = INT_MAX;
	int maxX = INT_MIN;
	int maxY = INT_MIN;

	Log::Instance()->Assert(!_soldiers.empty(),"Calculate center with no units!");

	for (auto& unit : _soldiers)
	{
		auto pos = unit->GetPointer()->getPosition();
		if (pos.x < minX) minX = pos.x;
		if (pos.x > maxX) maxX = pos.x;
		if (pos.y < minY) minY = pos.y;
		if (pos.y > maxY) maxY = pos.y;
	}

	_box->_topLeft = BWAPI::Position(minX, minY);
	_box->_bottomRight = BWAPI::Position(maxX, maxY);
	_box->_center = BWAPI::Position(minX + (maxX - minX) / 2, minY + (maxY - minY) / 2);
}

void Army::CheckTask()
{
	if (!_task)
		return;

	if (_task->IsFinished())
	{
		_task->Stop();
		_task = nullptr;
		return;
	}

	//check if worker defence is needed
	if (_task->Type() == Tasks::Type::DEFEND && (_task->EnemyArmy()->Supply() + 4) > GetSupply()) //TODO some random numbers here
	{
		ArmyModule::Instance()->StartWorkerDefence(_task, _task->EnemyArmy()->Supply() + 4 - GetSupply());
	}
}

Army::Army(bool air)
	:_task(nullptr), _bAir(air), _antiAir(0)
{
	_box = std::make_unique<Armies::Box>();
	_box->_topLeft = BWAPI::Position(0, 0);
	_box->_bottomRight = BWAPI::Position(0, 0);
	_box->_center = BWAPI::Position(0, 0);
}

Army::~Army()
{
	if (_task)
	{
		if (_task->Type() == Tasks::Type::SCOUT)
		{
			ArmyModule::Instance()->SetScoutTimeout(BWAPI::Broodwar->getFrameCount() + Config::Strategy::ScoutTimeout());
			
			Log::Instance()->Assert(_task->Area(), "No area in scout task!");
			for (auto& base : _task->Area()->Bases())
			{
				((BaseInfo*)base.Ptr())->_lastSeenFrame = BWAPI::Broodwar->getFrameCount();
				
				//try to scan base
				ScoutModule::Instance()->ScanBase(base);
			}
		}
			

		if (_task->Type() == Tasks::Type::ATTACK)
			ArmyModule::Instance()->AttackArmyKilled();

		_task->Stop();
	}

	if (ArmyModule::Instance()->DefaultAirTask()->FriendlyArmy() == this)
		ArmyModule::Instance()->ResetDefaultAirTask();
}

void Army::OnFrame()
{
	CalculateCenter();

	CheckTask();

	for (auto& unit : _soldiers)
	{
		unit->Fight(this);
	}

}

bool Army::AddSoldier(KasoBot::Unit* unit)
{
	if (_task 
		&& (_task->Type() == Tasks::Type::ATTACK || _task->Type() == Tasks::Type::SCOUT || _task->Type() == Tasks::Type::HARASS || _task->Type() == Tasks::Type::HUNT))
		return false;

	if (unit->GetPointer()->getType().isFlyer() && unit->GetPointer()->getType() != BWAPI::UnitTypes::Terran_Science_Vessel) //air units
	{
		if (!_bAir)
			return false;

		if (GetSupply() + unit->GetPointer()->getType().supplyRequired() > Config::Strategy::MinAirArmySupply())
			return false;
	}
	else //ground units
	{
		if (_bAir)
			return false;

		if (GetSupply() + unit->GetPointer()->getType().supplyRequired() > StrategyModule::Instance()->GetActiveStrat()->MinArmySupply())
			return false;
	}


	_soldiers.emplace_back(unit);

	if (unit->GetPointer()->getType().airWeapon() != BWAPI::WeaponTypes::None)
		_antiAir++;

	return true;
}

bool Army::SoldierKilled(KasoBot::Unit* unit)
{
	for (auto it = _soldiers.begin(); it != _soldiers.end(); it++)
	{
		if (*it == unit)
		{
			_soldiers.erase(it);
			if (unit->GetPointer()->getType().airWeapon() != BWAPI::WeaponTypes::None)
				_antiAir--;
			Log::Instance()->Assert(_antiAir >= 0, "Error in counting anti air units!");
			return true;
		}
	}

	return false;
}

int Army::GetSupply()
{
	int supply = 0;
	for (auto& unit : _soldiers)
	{
		supply += unit->GetPointer()->getType().supplyRequired();
	}

	return supply;
}

void Army::ClearTiles(BWAPI::TilePosition pos, BWAPI::UnitType type)
{
	for (auto& soldier : _soldiers)
	{
		int x = soldier->GetPointer()->getTilePosition().x;
		int y = soldier->GetPointer()->getTilePosition().y;

		if ((pos.x) - 1 <= x && x < (pos.x + type.tileWidth())
			&& (pos.y) - 1 <= y && y < (pos.y + type.tileHeight()))
			soldier->ClearTile();
	}
}

void Army::AssignTask(KasoBot::Task * task)
{
	Log::Instance()->Assert(task,"Task is nullptr when assigning!");

	if (_task)
		_task->Stop();

	_task = task;
	_task->Start();
}

void Army::RemoveTask()
{
	Log::Instance()->Assert(_task,"No task to remove!");
	_task->Stop();
	_task = nullptr;
}

Task * Army::Task()
{
	return _task ? _task : (_bAir ? ArmyModule::Instance()->DefaultAirTask() : ArmyModule::Instance()->DefaultTask());
}

KasoBot::Unit* Army::GetScoutSoldier()
{
	for (auto& soldier : _soldiers)
	{
		if (soldier->GetPointer()->getType() == BWAPI::UnitTypes::Terran_Marine
			|| soldier->GetPointer()->getType() == BWAPI::UnitTypes::Terran_Vulture
			|| soldier->GetPointer()->getType() == BWAPI::UnitTypes::Terran_Wraith)
			return soldier;
	}

	return nullptr;
}

BWAPI::Unit Army::GetRepairTarget()
{
	//TODO scv in army is not implemented so we just return nullptr for now
	return nullptr;
}

void WorkerArmy::CheckTask()
{
	if (!_task)
		return;

	if (_task->IsFinished())
	{
		_task->Stop();
		_task = nullptr;
		return;
	}
}

WorkerArmy::WorkerArmy()
	:Army(false)
{
}

WorkerArmy::~WorkerArmy()
{
}

std::vector<std::shared_ptr<Worker>> WorkerArmy::GetFreeWorkers(size_t max)
{
	std::vector<std::shared_ptr<Worker>> workers = {};
	
	if (max <= 0)
		return workers;

	//select workers to transfer
	for (auto worker : _workers)
	{
		if (worker->GetRole() == Units::Role::SCOUT
			|| worker->GetRole() == Units::Role::SCOUT_RUSH)
			continue;

		workers.emplace_back(worker);
		if (workers.size() >= max)
			break;
	}

	//remove selected from army
	_workers.erase(std::remove_if(_workers.begin(), _workers.end(),
		[workers](auto& x)
		{
			//cycle selected workers, erase if found
			for (auto worker : workers)
			{
				if (worker == x)
					return true;
			}
			return false;
		}
	), _workers.end());

	return workers;
}

void WorkerArmy::AddWorker(std::shared_ptr<Worker> worker)
{
	_workers.emplace_back(worker);

	//set few SCVs as bunker repairers
	if (ArmyModule::Instance()->Bunker())
	{
		int bunker = 0;
		for (auto& wrkr : _workers)
		{
			if (wrkr != worker && wrkr->GetRole() == Units::Role::BUNKER)
				bunker++;
		}
		if (bunker < Config::Strategy::BunkerWorkers())
			worker->SetRole(Units::Role::BUNKER);
		else
			worker->SetRole(Units::Role::IDLE);
		return;
	}
	
	worker->SetRole(_workers.size() % 2 == 1 || _workers.size() > 3 ? Units::Role::IDLE : Units::Role::REPAIR);
}

bool WorkerArmy::WorkerKilled(BWAPI::Unit unit)
{
	size_t before = _workers.size();

	_workers.erase(std::remove_if(_workers.begin(), _workers.end(),
		[unit](auto& x)
		{
			return unit == x->GetPointer();
		}
	), _workers.end());

	//check if worker was removed from list
	return before > _workers.size();
}

void WorkerArmy::OnFrame()
{	
	CheckTask();

	for (auto& worker : _workers)
	{
		if (worker->GetRole() == Units::Role::SCOUT)
		{
			worker->Scout();
		}
		else if (worker->GetRole() == Units::Role::SCOUT_RUSH)
		{
			worker->ScoutRush();
		}
		else
		{
			worker->Fight(this);
		}
	}
}

void WorkerArmy::RemoveTask()
{
	Army::RemoveTask();
}

BWAPI::Unit WorkerArmy::GetRepairTarget()
{
	BWAPI::Unit mostDamaged = nullptr;
	for (auto& worker : _workers)
	{
		//don't try to repair scouts
		if (worker->GetRole() != Workers::Role::IDLE
			&& worker->GetRole() != Workers::Role::REPAIR)
			continue;

		auto pointer = worker->GetPointer();
		if (pointer->getType().maxHitPoints() > pointer->getHitPoints() && (!mostDamaged
			|| (pointer->getType().maxHitPoints() - pointer->getHitPoints())
			> (mostDamaged->getType().maxHitPoints() - mostDamaged->getHitPoints())))
			mostDamaged = pointer;
	}
	return mostDamaged;
}
