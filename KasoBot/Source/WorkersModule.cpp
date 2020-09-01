#include "WorkersModule.h"
#include "ArmyModule.h"
#include "MapModule.h"

#include "Worker.h"
#include "Expansion.h"
#include "ProductionItem.h"

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
		if (!exp->GetPointer()->exists()) //this can happen when we are reassigning workers in expansion destructor
			break;

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
	//check if refinery is in this base and assign it
	AssignRefinery(exp);

	//get workers from army
	auto list = ArmyModule::Instance()->GetFreeWorkers(exp.IdealWorkerCount());

	for (auto worker : list)
	{
		exp.AddWorker(worker);
	}
	
	//TODO if not enough, get workers from oversaturated bases
}

void WorkersModule::AssignRefinery(Expansion& exp)
{
	for (auto it = _unassignedRefineries.begin(); it != _unassignedRefineries.end(); it++)
	{
		if (BWEM::Map::Instance().GetNearestArea((*it)->getTilePosition()) == exp.GetStation()->getBase()->GetArea())
		{
			exp.AddRefinery(*it);
			_unassignedRefineries.erase(it);
			break;
		}
	}
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
	for (auto& worker : _builders)
	{
		worker->Work();
	}
	for (auto& exp : _expansionList)
	{
		exp->OnFrame();
	}
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

void WorkersModule::MineralDestroyed(BWAPI::Unit unit)
{
	std::vector<BWAPI::Unit> workersToReassign;

	for (auto& exp : _expansionList)
	{
		if (exp->CheckMineral(unit, workersToReassign))
			break;
	}

	BWEM::Map::Instance().OnMineralDestroyed(unit); //remove mineral from BWEM::Map
	
	for(auto worker : workersToReassign)
		NewWorker(worker); //calling NewWorker from here instead of Expansion in case last mineral was mined, then workers should be transfered to another base
}

bool WorkersModule::Build(ProductionItem* item)
{
	_ASSERT(item->GetLocation() != BWAPI::TilePositions::Invalid);

	std::shared_ptr<Worker> closestWorker = nullptr;
	Expansion* expForWorker = nullptr; //saving expansion that have the closest worker, to easily remove him after
	int closestDist = INT_MAX;

	//select closest worker
	for (auto& exp : _expansionList)
	{
		for (auto& worker : exp->Workers())
		{
			//only mineral workers are building stuff for now
			if (worker->GetWorkerRole() == Workers::Role::MINERALS)
			{
				int dist = worker->GetPointer()->getDistance(BWAPI::Position(item->GetLocation()));

				if (dist < closestDist)
				{
					closestWorker = worker;
					closestDist = dist;
					expForWorker = exp.get();
				}					
			}
		}
	}

	if (closestWorker)
	{
		if (closestWorker->AssignRoleBuild(item))
		{
			_builders.emplace_back(closestWorker);
			expForWorker->RemoveWorker(closestWorker->GetPointer());
			return true;
		}
	}
	
	return false;
}

void WorkersModule::FinishBuild(BWAPI::Unit unit)
{
	for (auto it = _builders.begin(); it != _builders.end(); it++)
	{
		_ASSERT((*it)->GetProductionItem());

		if (unit->getTilePosition() == (*it)->GetProductionItem()->GetLocation())
		{
			_ASSERT(unit->getType() == (*it)->GetProductionItem()->GetType());

			(*it)->GetProductionItem()->Finish(); //finish task

			//reassign worker to mine
			NewWorker((*it)->GetPointer());
			_builders.erase(it);
			return;
		}
	}

	//there should never be building built without a builder in list
	_ASSERT(false);
}
