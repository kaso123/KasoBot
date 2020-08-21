#include "ArmyModule.h"
#include "Worker.h"

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
