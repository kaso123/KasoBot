#include "Army.h"
#include "Config.h"
#include "Task.h"
#include "ArmyModule.h"
#include "WorkersModule.h"
#include "EnemyArmy.h"
#include "Worker.h"

using namespace KasoBot;

void Army::CalculateCenter()
{
	int minX = INT_MAX;
	int minY = INT_MAX;
	int maxX = INT_MIN;
	int maxY = INT_MIN;

	_ASSERT(!_soldiers.empty());

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
	if (_task->Type() == Tasks::Type::DEFEND && (_task->EnemyArmy()->Supply() + 4) > GetSupply())
	{
		ArmyModule::Instance()->StartWorkerDefence(_task, _task->EnemyArmy()->Supply() + 4 - GetSupply());
	}
}

Army::Army()
	:_task(nullptr)
{
	_box = std::make_unique<Armies::Box>();
	_box->_topLeft = BWAPI::Position(0, 0);
	_box->_bottomRight = BWAPI::Position(0, 0);
	_box->_center = BWAPI::Position(0, 0);
}

Army::~Army()
{
	if (_task)
		_task->Stop();
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
		&& (_task->Type() == Tasks::Type::ATTACK || _task->Type() == Tasks::Type::SCOUT))
		return false;

	if (GetSupply() + unit->GetPointer()->getType().supplyRequired() > Config::Strategy::MaxArmySupply() * 2)
		return false;

	_soldiers.emplace_back(unit);
	return true;
}

bool Army::SoldierKilled(KasoBot::Unit* unit)
{
	for (auto it = _soldiers.begin(); it != _soldiers.end(); it++)
	{
		if (*it == unit)
		{
			_soldiers.erase(it);
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
	_ASSERT(task);

	if (_task)
		_task->Stop();

	_task = task;
	_task->Start();
}

void Army::RemoveTask()
{
	_ASSERT(_task);
	_task->Stop();
	_task = nullptr;
}

Task * Army::Task()
{
	return _task ? _task : ArmyModule::Instance()->DefaultTask();
}

void WorkerArmy::CheckTask()
{
	if (!_task)
		return;

	if (_task->IsFinished())
	{
		_task->Stop();
		_task = nullptr;
		WorkersModule::Instance()->AskForWorkers();
		return;
	}
}

WorkerArmy::WorkerArmy()
{
}

WorkerArmy::~WorkerArmy()
{
}

std::vector<std::shared_ptr<Worker>> WorkerArmy::GetFreeWorkers(size_t max)
{
	std::vector<std::shared_ptr<Worker>> workers = {};

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
	worker->SetRole(Units::Role::IDLE);

	//keep 2 SCVs on repair job
	if (ArmyModule::Instance()->Bunker())
	{
		if (_workers.size() <= 5) //TODO config
			worker->SetRole(Units::Role::BUNKER);
		else
			worker->SetRole(Units::Role::IDLE);
	}
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

	WorkersModule::Instance()->AskForWorkers();
}
