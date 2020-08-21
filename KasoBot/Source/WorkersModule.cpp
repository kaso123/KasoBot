#include "WorkersModule.h"
#include "ArmyModule.h"
#include "MapModule.h"

#include "Worker.h"
#include "Expansion.h"
#include "libs/BWEB/BWEB.h"

using namespace KasoBot;

WorkersModule* WorkersModule::_instance = 0;

WorkersModule::WorkersModule()
{
}

WorkersModule::~WorkersModule()
{
	delete(_instance);
}

Expansion* WorkersModule::FindExpansionForWorker(BWAPI::Unit unit)
{
	Expansion* closestFree = nullptr;
	Expansion* closest = nullptr;
	int distFree = INT_MAX;
	int dist = INT_MAX;

	for (const auto& exp : _expansionList)
	{
		int currDist = unit->getDistance(exp->GetPointer());
		if (currDist < dist && !exp->IsFull())
		{
			closest = exp.get();
			dist = unit->getDistance(exp->GetPointer());
		}
		if (currDist < distFree && !exp->IsSaturated())
		{
			closestFree = exp.get();
			distFree = unit->getDistance(exp->GetPointer());
		}
	}

	//try to find not saturated base first, then not oversaturated base
	if(closestFree)
		return closestFree;
	
	return closest;
}

void WorkersModule::AssignIdleWorkers(Expansion& exp)
{
	//get workers from army
	auto list = ArmyModule::Instance()->GetFreeWorkers(INT_MAX); //TODO add real limit

	for (auto worker : list)
	{
		exp.AddWorker(worker);
	}
	
	//TODO if not enough, get workers from oversaturated bases
}

WorkersModule* WorkersModule::Instance()
{
	if (!_instance)
		_instance = new WorkersModule;
	return _instance;
}

void WorkersModule::OnStart()
{

}

void WorkersModule::OnFrame()
{

}

void WorkersModule::NewWorker(BWAPI::Unit unit)
{
	//select expansion for worker
	Expansion* closest = FindExpansionForWorker(unit);
	if (!closest)
	{
		//send worker to military
		ArmyModule::Instance()->AddWorker(std::make_shared<Worker>(unit));
		return;
	}

	closest->AddWorker(unit);
}

void WorkersModule::RemoveWorker(BWAPI::Unit unit)
{
	//check if worker was from any expansion and remove him
	for (auto& exp : _expansionList)
	{
		if (exp->RemoveWorker(unit))
			return;
	}

	//if worker was not removed, it is part of army
	auto temp = ArmyModule::Instance()->WorkerKilled(unit);
	_ASSERT(temp); //every worker has to be in WorkerModule or army
}

void WorkersModule::ExpansionCreated(BWAPI::Unit unit)
{
	//create new expansion object and assign workers from army or oversaturated bases
	AssignIdleWorkers(*(_expansionList.emplace_back(std::make_unique<Expansion>(unit))));
}

void WorkersModule::ExpansionDestroyed(BWAPI::Unit unit)
{
	_expansionList.erase(std::remove_if(_expansionList.begin(), _expansionList.end(),
		[unit](auto& x) 
		{
			return unit == x->GetPointer();
		}
	),_expansionList.end());
}

void WorkersModule::RefineryCreated(BWAPI::Unit unit, bool unassign /*= false*/)
{
	if (!unassign)
	{
		//find which BWEB::Station this belongs to
		BWEB::Station* refineryStation = Map::GetStation(unit->getTilePosition());
		_ASSERT(refineryStation);

		for (auto& exp : _expansionList)
		{
			if (exp->GetStation() == refineryStation)
			{
				exp->AddRefinery(unit);
				return;
			}
		}
	}
	
	//if base is not taken then add to unassigned refineries
	_unassignedRefineries.emplace_back(unit);
}

void WorkersModule::RefineryDestroyed(BWAPI::Unit unit)
{
	for (auto& exp : _expansionList)
	{
		if (exp->RemoveRefinery(unit))
			return;
	}

	//TODO refinery wasn't in any expansion, must be in unassigned list
	size_t before = _unassignedRefineries.size();
	for (auto it = _unassignedRefineries.begin(); it != _unassignedRefineries.end(); it++)
	{
		if (*it == unit)
		{
			_unassignedRefineries.erase(it);
			return;
		}
	}

	_ASSERT(false); //we should never get here
}