#include "ArmyModule.h"
#include "Worker.h"
#include "Army.h"
#include "MapModule.h"
#include "ScoutModule.h"
#include "Config.h"
#include "Task.h"
#include "EnemyArmy.h"
#include "BaseInfo.h"

using namespace KasoBot;

ArmyModule* ArmyModule::_instance = 0;

ArmyModule::ArmyModule()
{
	_defaultTask = std::make_unique<HoldPositionTask>(Map::DefaultTaskPosition());
}

ArmyModule::~ArmyModule()
{
	delete(_instance);
}

void ArmyModule::AssignTasks()
{
	for (auto& task : _tasks)
	{
		if (task->InProgress())
			continue;

		for (auto& army : _armies)
		{
			if (army->Task() != _defaultTask.get())
				continue;

			if (!task->IsArmySuitable(*army))
				continue;

			army->AssignTask(task.get());
			break;
		}
	}
}

void ArmyModule::CreateAttackTasks()
{
	//count current attack tasks
	int count = 0;
	for (auto& task : _tasks)
	{
		if (task->Type() == Tasks::Type::ATTACK)
			count++;
	}

	if (count >= Config::Strategy::MaxAttackTasks()) 
		return;

	//find next area where enemies are
	for (auto& area : BWEM::Map::Instance().Areas())
	{
		bool enemy = false;
		for (auto& base : area.Bases())
		{
			if (((BaseInfo*)base.Ptr())->_owner == Base::Owner::ENEMY)
			{
				enemy = true;
				break;
			}
		}
		if (!enemy)
			continue;

		if (AddAttackTask(&area))
			return;
	}
}

void ArmyModule::CreateScoutTasks()
{
	//count current scout tasks
	int count = 0;
	for (auto& task : _tasks)
	{
		if (task->Type() == Tasks::Type::SCOUT)
			count++;
	}

	if (count >= Config::Strategy::MaxScoutTasks())
		return;

	const BWEM::Area* latest = nullptr;
	int latestFrame = INT_MAX;

	//find next area to scout
	for (auto& area : BWEM::Map::Instance().Areas())
	{
		for (auto& base : area.Bases())
		{
			if (((BaseInfo*)base.Ptr())->_owner == Base::Owner::UNKNOWN)
			{
				bool isInTask = false;
				for (auto& task : _tasks) //skip areas in other task
				{
					if (task->Area() == &area)
					{
						isInTask = true;
						break;
					}
				}
				if (isInTask)
					continue;
			 
				int frame = ((BaseInfo*)base.Ptr())->_lastSeenFrame; //choose last seen base
				if (latestFrame > frame)
				{
					latestFrame = frame;
					latest = &area;
				}
			}
		}	
	}

	if (latest)
		AddScoutTask(latest);
}

ArmyModule* ArmyModule::Instance()
{
	if (!_instance)
		_instance = new ArmyModule;
	return _instance;
}

void ArmyModule::OnFrame()
{
	//scouting with worker
	if (ScoutModule::Instance()->ShouldWorkerScout())
	{
		bool isScout = false;
		for (auto& worker : _workers)
		{
			if (worker->GetRole() == Units::Role::SCOUT)
			{
				isScout = true;
				break;
			}
		}
		if (!isScout && !_workers.empty())
			(_workers.front()->SetRole(Units::Role::SCOUT));
	}

	for (auto& worker : _workers)
	{
		if (worker->GetRole() == Units::Role::SCOUT)
			worker->Scout();
	}

	CreateAttackTasks();
	CreateScoutTasks();
	AssignTasks();

	for (auto& army : _armies)
	{
		army->OnFrame();
	}

	//remove selected from army
	_tasks.erase(std::remove_if(_tasks.begin(), _tasks.end(),
		[this](auto& x)
		{
			if (x->IsFinished())
			{
				if (x->InProgress())
				{
					for (auto& army : _armies)
					{
						if (army->Task() == x.get())
							army->RemoveTask();
					}
				}
				return true;
			}
			return false;
		}
	), _tasks.end());
}

std::vector<std::shared_ptr<Worker>> ArmyModule::GetFreeWorkers(size_t max)
{
	std::vector<std::shared_ptr<Worker>> workers = {};

	//TODO leave some workers for repair job when needed (when implemented)
	
	//select workers to transfer
	for (auto worker : _workers)
	{
		if (worker->GetRole() == Units::Role::SCOUT)
			continue;

		workers.emplace_back(worker);
		if (workers.size() >= max)
			break;
	}
	
	//remove selected from army
	_workers.erase(std::remove_if(_workers.begin(),_workers.end(),
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
	),_workers.end());

	return workers;
}

void ArmyModule::AddWorker(std::shared_ptr<Worker> worker)
{
	_workers.emplace_back(worker);
}

bool ArmyModule::WorkerKilled(BWAPI::Unit unit)
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

void ArmyModule::AddSoldier(KasoBot::Unit* unit)
{
	for (auto& army : _armies)
	{
		if (army->AddSoldier(unit))
		{
			return;
		}
	}

	auto& newArmy = _armies.emplace_back(std::make_unique<Army>());
	newArmy->AddSoldier(unit);
}

void ArmyModule::SoldierKilled(KasoBot::Unit* unit)
{

	for (auto it = _armies.begin(); it != _armies.end(); it++)
	{
		if ((*it)->SoldierKilled(unit))
		{
			if ((*it)->GetSupply() <= 0)
				_armies.erase(it);

			return;
		}
	}

	_ASSERT(false);
}

int ArmyModule::GetArmySupply()
{
	int supply = 0;

	//cycle through all armies
	for (auto& army : _armies)
	{
		supply += army->GetSupply();
	}

	supply += _workers.size();

	return supply;
}

void ArmyModule::ClearTiles(BWAPI::TilePosition pos, BWAPI::UnitType type)
{
	for (auto& army : _armies)
	{
		army->ClearTiles(pos,type);
	}
}

bool ArmyModule::NeedScout()
{
	if (_workers.size() <= 0)
		return true;
	return false;
}

bool ArmyModule::AddAttackTask(const BWEM::Area * area)
{
	_ASSERT(area);

	for (auto& task : _tasks)
	{
		if (task->Type() == Tasks::Type::ATTACK && task->Area() == area)
			return false;
	}
	_tasks.emplace_back(std::make_unique<AttackAreaTask>(area));
	return true;
}

bool ArmyModule::AddDefendTask(EnemyArmy* enemy)
{
	_ASSERT(enemy);

	for (auto& task : _tasks)
	{
		if (task->Type() == Tasks::Type::DEFEND && task->EnemyArmy() == enemy)
			return false;
	}

	_tasks.emplace_back(std::make_unique<DefendArmyTask>(enemy));
	return true;
}

bool ArmyModule::AddHoldTask(BWAPI::Position pos)
{
	_ASSERT(pos.isValid());

	for (auto& task : _tasks)
	{
		if (task->Type() == Tasks::Type::HOLD && task->Position() == pos)
			return false;
	}

	_tasks.emplace_back(std::make_unique<HoldPositionTask>(pos));
	return true;
}

bool ArmyModule::AddScoutTask(const BWEM::Area * area)
{
	_ASSERT(area);

	for (auto& task : _tasks)
	{
		if (task->Type() == Tasks::Type::SCOUT && task->Area() == area)
			return false;
	}
	_tasks.emplace_back(std::make_unique<ScoutAreaTask>(area));
	return true;
}

void ArmyModule::EnemyArmyRemoved(EnemyArmy * enemy)
{
	for (auto& army : _armies)
	{
		if (army->Task() && army->Task()->EnemyArmy() == enemy)
			army->RemoveTask();
	}

	//remove selected from army
	_tasks.erase(std::remove_if(_tasks.begin(), _tasks.end(),
		[enemy](auto& x)
		{
			if (x->EnemyArmy() == enemy)
				return true;

			return false;
		}
	), _tasks.end());
}

void ArmyModule::ResetDefaultTask()
{
	_defaultTask = std::make_unique<HoldPositionTask>(Map::DefaultTaskPosition());
}
