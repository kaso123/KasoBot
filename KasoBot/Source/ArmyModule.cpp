#include "ArmyModule.h"
#include "Worker.h"
#include "Army.h"
#include "MapModule.h"
#include "ScoutModule.h"
#include "StrategyModule.h"
#include "OwnStrategy.h"
#include "WorkersModule.h"
#include "Config.h"
#include "Task.h"
#include "EnemyArmy.h"
#include "BaseInfo.h"
#include "ProductionModule.h"
#include "Log.h"

using namespace KasoBot;

ArmyModule* ArmyModule::_instance = 0;

ArmyModule::ArmyModule()
	:_bunker(nullptr), _scoutTimeout(0)
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
	//sort tasks, defend > attack > scout
	std::sort(std::begin(_tasks), std::end(_tasks),
		[](const std::unique_ptr<Task>& a, const std::unique_ptr<Task>&  b)
		{
			if (a->Type() == Tasks::Type::ATTACK && b->Type() == Tasks::Type::ATTACK)
			{
				if (a->Area() == ScoutModule::Instance()->EnemyStart()) //enemy main always last
					return false;
				if (a->Area() == ScoutModule::Instance()->EnemyNatural()) //enemy nat only before enemy main
				{
					if (b->Area() == ScoutModule::Instance()->EnemyStart())
						return true;
					return false;
				}
				
				return a->Area()->Bases().front().Center().getApproxDistance(BWEB::Map::getMainPosition())
					< b->Area()->Bases().front().Center().getApproxDistance(BWEB::Map::getMainPosition());
			}
				
			return a->Type() < b->Type();
		});

	int armySupply = GetArmySupply(false);

	for (auto& task : _tasks)
	{
		if (task->InProgress())
			continue;
		if (task->Type() == Tasks::Type::SCOUT
			&& (armySupply < 16 || _scoutTimeout > BWAPI::Broodwar->getFrameCount())) //TODO configurable
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

		//no army found for scout task
		if (task->Type() == Tasks::Type::SCOUT)
		{
			//try to split an idle army
			SplitArmyForScout(task.get());
		}
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

	if (count >= StrategyModule::Instance()->GetActiveStrat()->MaxAttackTasks()) 
		return;


	//sort enemy areas by distance
	std::vector<std::pair<const BWEM::Base*,int>> enemyBases;
	//find next area where enemies are
	for (auto& area : BWEM::Map::Instance().Areas())
	{
		if (!Map::CanAccess(&area))
			continue;

		for (auto& base : area.Bases())
		{
			if (((BaseInfo*)base.Ptr())->_owner == Base::Owner::ENEMY)
			{
				int dist = base.Center().getApproxDistance(BWEB::Map::getMainPosition());
				if (&area == ScoutModule::Instance()->EnemyStart())
					dist += BWAPI::Broodwar->mapHeight() * 32;
				if (&area == ScoutModule::Instance()->EnemyNatural())
					dist += BWAPI::Broodwar->mapHeight() * 16;

				enemyBases.emplace_back(std::make_pair(&base,dist));
				break;
			}
		}
	}

	std::sort(std::begin(enemyBases), std::end(enemyBases),
		[](const std::pair<const BWEM::Base*, int>& a, const std::pair<const BWEM::Base*, int>&  b)
		{
			return a.second < b.second;
		});

	count = 1;
	while (true)
	{
		//find next area where enemies are
		for (auto& base : enemyBases)
		{
			if (AddAttackTask(base.first->GetArea(),count))
				return;
		}
		count++;
		if (count > Config::Strategy::MaxTasksPerArea())
			break;
	}

	//no attack task could be created, create finish task if not already
	AddFinishTask();
}

void ArmyModule::CreateScoutTasks()
{
	if (BWAPI::Broodwar->getFrameCount() < Config::Strategy::ScoutTasksStart())
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

void ArmyModule::SplitArmyForScout(Task * task)
{
	Log::Instance()->Assert(task->Type() == Tasks::Type::SCOUT, "Army splitting for non-scout task!");

	for (auto& army : _armies)
	{
		if (army->Task() && army->Task()->Type() == Tasks::Type::HOLD)
		{
			//get one unit that can scout
			auto soldier = army->GetScoutSoldier();
			if (!soldier)
				continue;

			army->SoldierKilled(soldier);
			auto newArmy = _armies.emplace_back(std::make_unique<Army>()).get();
			Log::Instance()->Assert(newArmy != nullptr, "No army created for scout soldier!");
			newArmy->AddSoldier(soldier);
			newArmy->AssignTask(task);
			return;
		}
	}
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

	//if there are idle workers in army try to move them to work
	if (_workers->Task() == _defaultTask.get())
	{
		for (auto& worker : _workers->Workers())
		{
			if (worker->GetRole() == Units::Role::IDLE)
			{
				WorkersModule::Instance()->AskForWorkers();
				break;
			}
		}
	}
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

	Log::Instance()->Assert(false,"Soldier was not from any army when killed!");
}

int ArmyModule::GetArmySupply(bool countWorkers/*=true*/)
{
	int supply = 0;

	//cycle through all armies
	for (auto& army : _armies)
	{
		supply += army->GetSupply();
	}

	if(countWorkers)
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
	Log::Instance()->Assert(area,"Attack task area doesn't exist!");

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
	Log::Instance()->Assert(enemy, "Enemy army missing in Defend task!");
	
	if (enemy->IsWorkerRush()) //moving default task to CC when worker rushed
		ResetDefaultTask();

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
	Log::Instance()->Assert(pos.isValid(),"Invalid position for Hold task!");

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
	Log::Instance()->Assert(area, "Scout task area doesn't exist!");

	for (auto& task : _tasks)
	{
		if (task->Type() == Tasks::Type::SCOUT && task->Area() == area)
			return false;
	}
	_tasks.emplace_back(std::make_unique<ScoutAreaTask>(area));
	return true;
}

bool ArmyModule::AddFinishTask()
{
	if (GetArmySupply() < 100) //TODO configurable?
		return false;

	for (auto& task : _tasks)
	{
		if (task->Type() == Tasks::Type::FINISH || task->Type() == Tasks::Type::ATTACK)
			return false;
	}
	_tasks.emplace_back(std::make_unique<FinishEnemyTask>());
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

	if (!_bunker)
		return;

	//move bunker to natural
	if (_bunker->GetPointer()->getTilePosition() != (BWAPI::TilePosition)_defaultTask->Position())
	{
		//TODO check if bunker is already there

		_bunker->GetPointer()->unloadAll();
		_bunker = nullptr;
		ProductionModule::Instance()->BuildBuilding(BWAPI::UnitTypes::Terran_Bunker);
	}
		
}

void ArmyModule::StartWorkerDefence(Task * task, size_t count)
{
	Log::Instance()->Assert(task->Type() == Tasks::Type::DEFEND,"Worker defence started for non-defence task!");

	if (task->EnemyArmy()->IsOnlyFlying())
		return;

	if (_bunker)
		count = Config::Strategy::BunkerWorkers();
	else if (_workers->Task() != task && _workers->Task()->EnemyArmy() && _workers->Task()->EnemyArmy()->IsCannonRush())
	{
		count += _workers->Task()->EnemyArmy()->Supply();
	}

	count = std::min(count, (size_t)8); //TODO config
	count += 2; //add scouting scvs that are not defending
	//start worker defence
	if(count + 2 > _workers->Workers().size())
		WorkersModule::Instance()->WorkerDefence(count - _workers->Workers().size() + 2);
	if(_workers->Task() == ArmyModule::Instance()->DefaultTask())
		_workers->AssignTask(task);
}

void ArmyModule::ResetAttackTasks()
{
	//remove unassigned attack tasks
	_tasks.erase(std::remove_if(std::begin(_tasks), std::end(_tasks),
		[](auto& x)
		{
			return x->Type() == Tasks::Type::ATTACK && !x->InProgress();
		}
	), std::end(_tasks));
}

bool ArmyModule::IsCloseToAnyArmy(BWAPI::Unit unit)
{
	if (!unit)
		return false;

	for (auto& army : _armies)
	{
		if (unit->getDistance(army->BoundingBox()._center) < (Config::Units::ArmyRange() + Config::Units::EnemyArmyRange()) * 32)
			return true;
	}

	return false;
}
