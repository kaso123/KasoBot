#include "ArmyModule.h"
#include "Worker.h"
#include "Army.h"
#include "MapModule.h"
#include "ScoutModule.h"
#include "WorkersModule.h"
#include "Config.h"
#include "Task.h"
#include "EnemyArmy.h"
#include "BaseInfo.h"

using namespace KasoBot;

ArmyModule* ArmyModule::_instance = 0;

ArmyModule::ArmyModule()
	:_bunker(nullptr)
{
	_defaultTask = std::make_unique<HoldPositionTask>(Map::DefaultTaskPosition());
	_workers = std::make_unique<KasoBot::WorkerArmy>();
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
		if (task->InProgress()) //task was assigned  to army, worker defence is called from there
			continue;

		if (task->Type() == Tasks::Type::DEFEND) //we didnt find army to defend
		{
			//start worker defence
			StartWorkerDefence(task.get(),task->EnemyArmy()->Supply() + 1);
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

	count = 1;
	while (true)
	{
		//find next area where enemies are
		for (auto& area : BWEM::Map::Instance().Areas())
		{
			if (!Map::CanAccess(&area))
				continue;

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

			if (AddAttackTask(&area,count))
				return;
		}
		count++;
		if (count > 3) //TODO make configurable max att tasks per area
			return;
	}
}

void ArmyModule::CreateScoutTasks()
{
	if (BWAPI::Broodwar->getFrameCount() < 8 * 24 * 60) //TODO 8 minutes, make configurable
		return;

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
		if (!Map::CanAccess(&area))
			continue;

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
	if (ScoutModule::Instance()->ShouldWorkerScout() || ScoutModule::Instance()->ShouldWorkerScoutRush())
	{
		bool isScout = false;
		bool isScoutRush = false;
		for (auto& worker : _workers->Workers())
		{
			if (worker->GetRole() == Units::Role::SCOUT)
			{
				isScout = true;
			}
			if (worker->GetRole() == Units::Role::SCOUT_RUSH)
			{
				isScoutRush = true;
			}
		}
		if (!isScout && ScoutModule::Instance()->ShouldWorkerScout())
		{
			for (auto& worker : _workers->Workers())
			{
				if (worker->GetRole() == Units::Role::IDLE)
				{
					worker->SetRole(Units::Role::SCOUT);
					break;
				}
			}
		}
		if (!isScoutRush && ScoutModule::Instance()->ShouldWorkerScoutRush())
		{
			for (auto& worker : _workers->Workers())
			{
				if (worker->GetRole() == Units::Role::IDLE)
				{
					worker->SetRole(Units::Role::SCOUT_RUSH);
					break;
				}
			}
		}
	}

	CreateAttackTasks();
	CreateScoutTasks();
	AssignTasks();

	_workers->OnFrame();

	for (auto& army : _armies)
	{
		army->OnFrame();
	}

	//remove finished tasks
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
					if (_workers->Task() == x.get())
						_workers->RemoveTask();
				}
				return true;
			}
			return false;
		}
	), _tasks.end());
}

std::vector<std::shared_ptr<Worker>> ArmyModule::GetFreeWorkers(size_t max)
{
	return _workers->GetFreeWorkers(max);
}

void ArmyModule::AddWorker(std::shared_ptr<Worker> worker)
{
	_workers->AddWorker(worker);
}

bool ArmyModule::WorkerKilled(BWAPI::Unit unit)
{
	return _workers->WorkerKilled(unit);
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

	supply += _workers->Workers().size();

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
	for (auto& worker : _workers->Workers())
	{
		if (worker->GetRole() == Units::Role::IDLE
			|| worker->GetRole() == Units::Role::SCOUT)
			return false;
	}
	return true;
}

bool ArmyModule::NeedScoutRush()
{
	for (auto& worker : _workers->Workers())
	{
		if (worker->GetRole() == Units::Role::IDLE
			|| worker->GetRole() == Units::Role::SCOUT_RUSH)
			return false;
	}
	return true;
}

bool ArmyModule::AddAttackTask(const BWEM::Area * area, int limit /*=1*/)
{
	_ASSERT(area);

	int found = 0;
	for (auto& task : _tasks)
	{
		if (task->Type() == Tasks::Type::ATTACK && task->Area() == area)
			found++;
	}
	if (found >= limit)
		return false;

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
	if (_workers->Task() && _workers->Task()->EnemyArmy() == enemy)
		_workers->RemoveTask();

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

void ArmyModule::StartWorkerDefence(Task * task, size_t count)
{
	_ASSERT(task->Type() == Tasks::Type::DEFEND);

	if (_bunker)
		count = 4; //TODO configurable

	//start worker defence
	if(count > _workers->Workers().size())
		WorkersModule::Instance()->WorkerDefence(count - _workers->Workers().size());
	_workers->AssignTask(task);
}
