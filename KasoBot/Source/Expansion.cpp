#include "Expansion.h"
#include "MapModule.h"
#include "WorkersModule.h"
#include "StrategyModule.h"
#include "ProductionModule.h"
#include "ArmyModule.h"
#include "Worker.h"
#include "Config.h"
#include "BaseInfo.h"
#include "Log.h"

#include <algorithm>

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

	Log::Assert(_station,"No station found for expansion!");

	((BaseInfo*)_station->getBWEMBase()->Ptr())->_owner = Base::Owner::PLAYER;
	ArmyModule::Instance()->ResetDefaultTask();
}

Expansion::~Expansion()
{
	//transfer workers to another expansion / army
	for (auto worker : _workerList)
	{
		//add new worker, it should not add them back to this expansion (handled in WorkersModule)
		//no need to delete workers, they get deleted automatically
		WorkersModule::Instance()->NewWorker(worker->GetPointer());
	}


	if (_refinery)
	{
		//add refinery to _unassignedRefineries in WorkersModule
		WorkersModule::Instance()->RefineryCreated(_refinery, true);
		_refinery = nullptr;
	}

	Log::Assert(_station,"No station in expansion destructor!");
	((BaseInfo*)_station->getBWEMBase()->Ptr())->_owner = Base::Owner::NONE;
	ArmyModule::Instance()->ResetDefaultTask();
}

void Expansion::OnFrame()
{
	for (auto& worker : _workerList)
	{
		worker->Work();
	}
}

void Expansion::AddWorker(BWAPI::Unit unit)
{
	AddWorker(std::make_shared<Worker>(unit));
}

void Expansion::AddWorker(std::shared_ptr<Worker> worker)
{
	_workerList.emplace_back(worker);

	//refinery not saturated
	if (_refinery && _workersGas < Config::Workers::MaxPerGas() && _workersMinerals >= Config::Workers::StartGasAfter())
	{
		//assign role to worker / assign refinery
		worker->AssignRoleGas(_refinery);

		_workersGas++;
	}
	else //refinery is saturated (or doesn't exists)
	{
		//choose mineral
		BWEM::Mineral* mineral = Map::NextMineral(_station->getBWEMBase());

		Log::Assert(mineral,"No mineral for worker found!");

		//assign to worker
		worker->AssignRoleMinerals(mineral);
		
		_workersMinerals++;
	}

	Log::Assert(VerifyWorkers(),"Wrong worker count after add!");

	//check if refinery should be built, don't interfere with opener
	if (!_refinery && !StrategyModule::Instance()->IsOpenerActive() && !_station->getBWEMBase()->Geysers().empty())
	{
		if(_workersMinerals >= Config::Workers::StartGasAfter() && !ProductionModule::Instance()->IsInQueue(BWAPI::UnitTypes::Terran_Refinery))
			ProductionModule::Instance()->BuildRefineryAtExpansion(*this);
	}
}

void Expansion::AddRefinery(BWAPI::Unit unit)
{
	_refinery = unit;

	//only add workers to gas if we have enough mineral workers and don't add more than specified in config
	int toGas = std::max(_workersMinerals - Config::Workers::StartGasAfter(), Config::Workers::MaxPerGas());
	if (toGas < 0)
		return;

	std::vector<BWAPI::Unit> workersToGas;

	//select workers for gas mining
	for (toGas; toGas >= 0; toGas--) //only send workers above limit
	{
		//loop from back
		for (auto it = _workerList.rbegin(); it != _workerList.rend(); ++it)
		{
			if ((*it)->GetWorkerRole() == Workers::Role::MINERALS)
			{
				workersToGas.emplace_back(_workerList.back()->GetPointer());
				RemoveWorker((*it)->GetPointer());
				break;
			}
		}
	}

	//add workers back to expansion
	for (auto worker : workersToGas)
	{
		AddWorker(worker);
	}
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

	Log::Assert(VerifyWorkers(),"Wrong worker count after removing!");

	//check if worker was removed from list
	return before > _workerList.size();
}

bool Expansion::RemoveRefinery(BWAPI::Unit unit)
{
	if (_refinery == unit)
	{
		_refinery = nullptr;
		
		if(_workersGas <= 0) //no workers in gas
			return true;

		std::vector<BWAPI::Unit> toReassign;

		for (auto it = _workerList.begin(); it != _workerList.end(); it++)
		{
			if ((*it)->GetWorkerRole() == Workers::Role::GAS)
				toReassign.emplace_back((*it)->GetPointer());
		}

		for (auto worker : toReassign)
		{
			RemoveWorker(worker);
			WorkersModule::Instance()->NewWorker(worker); //going through workersModule in case expansion is full
		}
		return true;
	}

	return false;
}

bool Expansion::IsSaturated()
{
	if (_refinery && _workersGas < Config::Workers::SaturationPerGas())
		return false;

	//no minerals left so exp is full with 0 workers
	if (_station->getBWEMBase()->Minerals().empty())
		return true;

	if ((int)_station->getBWEMBase()->Minerals().size() * Config::Workers::SaturationPerMineral() > _workersMinerals)
		return false;

	return true;
}

bool Expansion::IsFull()
{
	if (_refinery && _workersGas < Config::Workers::MaxPerGas())
		return false;

	//no minerals left so exp is full with 0 workers
	if (_station->getBWEMBase()->Minerals().empty())
		return true;

	if ((int)_station->getBWEMBase()->Minerals().size() * Config::Workers::MaxPerMineral() > _workersMinerals)
		return false;

	return true;
}

bool Expansion::CheckMineral(BWAPI::Unit mineral, std::vector<BWAPI::Unit>& outToRemove)
{
	//mineral is not from this base
	if (BWEM::Map::Instance().GetNearestArea(mineral->getTilePosition()) != _station->getBWEMBase()->GetArea())
		return false;

	//select workers to reassign
	for (auto& worker : _workerList)
	{
		if (worker->IsMiningMineral(mineral))
		{
			outToRemove.emplace_back(worker->GetPointer());
		}
	}

	//reassign workers that had this mineral
	for (auto worker : outToRemove)
		RemoveWorker(worker);

	return true;
}

size_t Expansion::IdealWorkerCount()
{
	if (!_station)
		return 0;

	//check if we have more than enough workers to avoid size_t being negative
	if(_station->getBWEMBase()->Minerals().size() * Config::Workers::SaturationPerMineral() <= (size_t)_workersMinerals
		&& (!_refinery || Config::Workers::SaturationPerGas() <= _workersGas))
		return 0;

	return _station->getBWEMBase()->Minerals().size() * Config::Workers::SaturationPerMineral() 
		+ (_refinery ? Config::Workers::SaturationPerGas() : 0)
		- _workersGas - _workersMinerals;
}

std::vector<std::shared_ptr<Worker>> Expansion::GetUnneededWorkers(size_t max)
{
	std::vector<std::shared_ptr<Worker>> workers = {};

	//select workers to transfer
	for (auto& worker : _workerList)
	{
		if (workers.size() >= max)
			break;
		if (!worker->GetMineral() || worker->GetMineral()->Data() <= Config::Workers::SaturationPerMineral())
			continue;

		worker->RemoveMineral();

		workers.emplace_back(worker);
	}

	//remove selected from army
	_workerList.erase(std::remove_if(_workerList.begin(), _workerList.end(),
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
	), _workerList.end());

	_workersMinerals -= workers.size();

	Log::Assert(VerifyWorkers(),"Wrong worker count after transfering unneeded!");
	return workers;
}
