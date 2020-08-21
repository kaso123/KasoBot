#include "Expansion.h"
#include "MapModule.h"
#include "Worker.h"
#include "Config.h"

using namespace KasoBot;

bool Expansion::VerifyWorkers()
{
	return _workerList.size() == _workersMinerals + _workersGas;
}

Expansion::Expansion(BWAPI::Unit unit)
	:_pointer(unit), _station(nullptr), _refinery(nullptr),_workersMinerals(0), _workersGas(0)
{
	//find which station this expansion belongs to
	_station = KasoBot::Map::GetStation(unit->getTilePosition());

	_ASSERT(_station);
}

Expansion::~Expansion()
{
	//TODO transfer workers to another expansion / army
	//TODO add refinery to _unassignedRefineries in WorkersModule
}

void Expansion::AddWorker(BWAPI::Unit unit)
{
	AddWorker(std::make_shared<Worker>(unit));
}

void Expansion::AddWorker(std::shared_ptr<Worker> worker)
{
	_workerList.emplace_back(worker);

	//refinery not saturated
	if (_refinery && _workersGas < Config::Workers::MaxPerGas())
	{
		//assign role to worker / assign refinery
		worker->AssignRoleGas(_refinery);

		_workersGas++;
	}
	else //refinery is saturated (or doesn't exists)
	{
		//choose mineral
		BWEM::Mineral* mineral = Map::NextMineral(_station->getBase());

		_ASSERT(mineral);

		//assign to worker
		worker->AssignRoleMinerals(mineral);
		
		_workersMinerals++;
	}

	_ASSERT(VerifyWorkers());
}

void Expansion::AddRefinery(BWAPI::Unit unit)
{
	_refinery = unit;

	//TODO check if workers can be assigned to gas mining
}

bool Expansion::RemoveWorker(BWAPI::Unit unit)
{
	size_t before = _workerList.size();

	for (auto it = _workerList.begin(); it != _workerList.end(); it++)
	{
		if ((*it)->GetPointer() != unit)
			continue;

		//decrease counter according to role
		if ((*it)->GetWorkerRole() == Workers::Role::GAS)
			_workersGas--;
		else _workersMinerals--;

		_workerList.erase(it);
		break;
	}

	_ASSERT(VerifyWorkers());

	//check if worker was removed from list
	return before > _workerList.size();
}

bool Expansion::RemoveRefinery(BWAPI::Unit unit)
{
	if (_refinery == unit)
	{
		_refinery = nullptr;
		//TODO transfer workers to minerals
		return true;
	}

	return false;
}

bool Expansion::IsSaturated()
{
	if (!_refinery || _workersGas < Config::Workers::SaturationPerGas())
		return false;

	if ((int)_station->getBase()->Minerals().size() * Config::Workers::SaturationPerMineral() > _workersMinerals)
		return false;

	return true;
}

bool Expansion::IsFull()
{
	if (!_refinery || _workersGas < Config::Workers::MaxPerGas())
		return false;

	if ((int)_station->getBase()->Minerals().size() * Config::Workers::MaxPerMineral() > _workersMinerals)
		return false;

	return true;
}
