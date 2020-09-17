#include "ArmyModule.h"
#include "Worker.h"
#include "Army.h"

using namespace KasoBot;

ArmyModule* ArmyModule::_instance = 0;

ArmyModule::ArmyModule()
{
}

ArmyModule::~ArmyModule()
{
	delete(_instance);
}

ArmyModule* ArmyModule::Instance()
{
	if (!_instance)
		_instance = new ArmyModule;
	return _instance;
}

std::vector<std::shared_ptr<Worker>> ArmyModule::GetFreeWorkers(size_t max)
{
	std::vector<std::shared_ptr<Worker>> workers = {};

	//TODO leave some workers for repair job when needed (when implemented)
	
	//select workers to transfer
	for (auto worker : _workers)
	{
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
