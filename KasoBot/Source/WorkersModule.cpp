#include "WorkersModule.h"
#include "ArmyModule.h"
#include "MapModule.h"
#include "ScoutModule.h"
#include "ProductionModule.h"
#include "StrategyModule.h"
#include "Config.h"
#include "Log.h"

#include "Worker.h"
#include "Expansion.h"
#include "ProductionItem.h"
#include "Army.h"

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
		if (!exp || !exp->GetPointer() || !exp->GetPointer()->exists()) //this can happen when we are reassigning workers in expansion destructor
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

	size_t wantWorkers = exp.IdealWorkerCount();
	if (wantWorkers == 0)
		return;
	
	//get workers from army
	auto list = ArmyModule::Instance()->GetFreeWorkers(wantWorkers);
	
	for (auto worker : list)
	{
		exp.AddWorker(worker);
	}
	
	//if not enough, get workers from oversaturated bases
	if (list.size() < wantWorkers)
	{
		size_t added = 0; //total count of workers from other bases
		
		for (auto& base : _expansionList)
		{
			if (base.get() == &exp)
				continue;
			if (list.size() + added >= wantWorkers)
				break;

			if (!base->IsSaturated())
				continue;
			
			auto workers = base->GetUnneededWorkers(wantWorkers - list.size() - added);
			for (auto& worker : workers)
				exp.AddWorker(worker);
			added += workers.size();
		}
	}
}

void WorkersModule::AssignRefinery(Expansion& exp)
{
	if (exp.GetRefinery())
		return;

	for (auto it = _unassignedRefineries.begin(); it != _unassignedRefineries.end(); it++)
	{
		Log::Instance()->Assert((*it)->getTilePosition().isValid(), "Unassigned refinery has invalid position!");
		if (!(*it)->getTilePosition().isValid())
			continue;

		if (BWEM::Map::Instance().GetNearestArea((*it)->getTilePosition()) == exp.GetStation()->getBWEMBase()->GetArea())
		{
			exp.AddRefinery(*it);
			_unassignedRefineries.erase(it);
			break;
		}
	}
}

void WorkersModule::HandleRepairs()
{
	auto repairBuildings = ProductionModule::Instance()->GetDamagedBuildings();

	//only keep one worker per building
	while (repairBuildings.size() < _repairers.size())
	{
		for (auto it = _repairers.begin(); it != _repairers.end(); it++)
		{
			bool stays = false;
			for (auto& building : repairBuildings) //keep those that targets buildings from list
			{
				if ((*it)->IsRepairing(building))
				{
					stays = true;
					break;
				}
			}
			if (!stays)
			{
				NewWorker((*it)->GetPointer());
				_repairers.erase(it);
				break;
			}
		}
		Log::Instance()->Assert(false, "Multiple repairers on the same building!");
		break;
	}

	//get more workers if needed

	for (auto& building : repairBuildings) //check if every building is repaired, if not try to get closest worker
	{
		bool hasWorker = false;
		for (auto& worker : _repairers)
		{
			if (worker->IsRepairing(building))
			{
				hasWorker = true;
				break;
			}
		}
		if (hasWorker)
			continue;

		//try workers from list, if none is free get new worker
		for (auto& worker : _repairers)
		{
			bool free = true;
			for (auto& b : repairBuildings)
			{
				if (worker->IsRepairing(b))
				{
					free = false;
					break;
				}
			}

			if (!free)
				continue;

			worker->AssignRoleRepair(building);
			hasWorker = true;
			break;
		}

		if (hasWorker)
			continue;

		int keep = 3; //TODO config value together with WorkerDefence

		//get free worker from closest expansion
		Expansion* closest = nullptr;
		double closestDist = DBL_MAX;
		for (auto& exp : _expansionList) //select closest exp with enough workers
		{
			if (exp->WorkerCountMinerals() <= keep)
				continue;

			if (exp->GetPointer()->getDistance(building) < closestDist)
			{
				closestDist = exp->GetPointer()->getDistance(building);
				closest = exp.get();
			}
		}

		if (closest)
		{
			Log::Instance()->Assert(!closest->Workers().empty(), "Empty expansion chosen for repair job!");
			auto worker = _repairers.emplace_back(closest->Workers().front());
			worker->AssignRoleRepair(building);
			closest->RemoveWorker(worker->GetPointer());
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
	HandleRepairs();

	for (auto& worker : _builders)
	{
		worker->Work();
	}
	for (auto& worker : _repairers)
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
	//check if this worker should be send to scout, if so send him to the army
	if (ScoutModule::Instance()->ShouldWorkerScout() && ArmyModule::Instance()->NeedScout()
		|| ScoutModule::Instance()->ShouldWorkerScoutRush() && ArmyModule::Instance()->NeedScoutRush())
	{
		ArmyModule::Instance()->AddWorker(std::make_shared<Worker>(unit));
		return;
	}

	//select expansion for worker
	Expansion* closest = FindExpansionForWorker(unit);
	if (!closest) //all expansions full
	{
		//send worker to military
		ArmyModule::Instance()->AddWorker(std::make_shared<Worker>(unit));
		return;
	}

	closest->AddWorker(unit);
}

void WorkersModule::RemoveWorker(BWAPI::Unit unit)
{
	if (!unit->isCompleted()) //when CC is destroyed while producing worker, the worker is destroyed also
	{
		return;
	}
		

	//check if worker was from any expansion and remove him
	for (auto& exp : _expansionList)
	{
		if (exp->RemoveWorker(unit))
			return;
	}

	for (auto it = _builders.begin(); it != _builders.end(); it++)
	{
		if ((*it)->GetPointer() == unit)
		{
			_builders.erase(it);
			return;
		}
	}

	for (auto it = _repairers.begin(); it != _repairers.end(); it++)
	{
		if ((*it)->GetPointer() == unit)
		{
			_repairers.erase(it);
			return;
		}
	}

	//if worker was not removed, it is part of army
	auto temp = ArmyModule::Instance()->WorkerKilled(unit);
	Log::Instance()->Assert(temp,"Dead worker was not found anywhere!"); //every worker has to be in WorkerModule or army
}

bool WorkersModule::BuildWorker()
{
	for (auto& exp : _expansionList)
	{
		if (exp->GetPointer()->isIdle() && !exp->IsLocked())
		{
			if (exp->GetPointer()->train(BWAPI::UnitTypes::Terran_SCV))
				return true;
		}
	}
	return false;
}

void WorkersModule::ExpansionCreated(BWAPI::Unit unit)
{
	//create new expansion object and assign workers from army or oversaturated bases
	AssignIdleWorkers(*(_expansionList.emplace_back(std::make_unique<Expansion>(unit))));
}

void WorkersModule::ExpansionDestroyed(BWAPI::Unit unit)
{
	if (!unit->isCompleted())
		return;

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
		Log::Instance()->Assert(refineryStation,"No station found for refinery!");

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
	if (!unit->isCompleted())
		return;

	for (auto& exp : _expansionList)
	{
		if (exp->RemoveRefinery(unit))
			return;
	}

	//refinery wasn't in any expansion, must be in unassigned list
	size_t before = _unassignedRefineries.size();
	for (auto it = _unassignedRefineries.begin(); it != _unassignedRefineries.end(); it++)
	{
		if (*it == unit)
		{
			_unassignedRefineries.erase(it);
			return;
		}
	}

	Log::Instance()->Assert(false,"Destroyed refinery was not found!"); //we should never get here
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
	Log::Instance()->Assert(item->GetLocation().isValid(),"Build location is invalid!");

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

bool WorkersModule::BuildAddon(BWAPI::UnitType type)
{
	Log::Instance()->Assert(type.whatBuilds().first.isResourceDepot(),"Wrong addon send to workersModule!");

	for (auto& exp : _expansionList)
	{
		if (exp->IsLocked()) //only one locked building of one type at each time
		{
			if (exp->GetPointer()->getAddon())
			{
				Log::Instance()->Assert(false, "Building with addon is still locked!"); //shouldn't happen
				exp->Unlock();
				return false;
			}
			if (exp->GetPointer()->isIdle())
			{
				if (exp->GetPointer()->buildAddon(type)) //build started
				{
					exp->Unlock();
					return true;
				}
			}
			return false;
		}
	}

	for (auto& exp : _expansionList)
	{
		if (!exp->GetPointer()->getAddon() && !exp->GetPointer()->isConstructing() && !exp->IsLocked())
		{
			exp->Lock();
			return false;
		}
	}
	return false;
}

void WorkersModule::FinishBuild(BWAPI::Unit unit)
{
	for (auto it = _builders.begin(); it != _builders.end(); it++)
	{
		Log::Instance()->Assert((*it)->GetProductionItem(),"Builder has no prod item assinged!");

		if (unit->getTilePosition() == (*it)->GetProductionItem()->GetLocation())
		{
			Log::Instance()->Assert(unit->getType() == (*it)->GetProductionItem()->GetType(),"Builder was building sth else on tile!");

			(*it)->GetProductionItem()->Finish(); //finish task
			(*it)->BuildFinished();

			//reassign worker to mine
			NewWorker((*it)->GetPointer());
			_builders.erase(it);
			return;
		}
	}

	//there should never be building built without a builder in list
	Log::Instance()->Assert(false,"No builder for finished building!");
}

void WorkersModule::BuildFailed(ProductionItem * item)
{
	//find worker that was building this
	for (auto it = _builders.begin(); it != _builders.end(); it++)
	{
		if ((*it)->GetProductionItem() == item)
		{
			Log::Instance()->Assert((*it)->GetWorkerRole() == Workers::Role::BUILD || (*it)->GetWorkerRole() == Workers::Role::ASSIGNED, "Build failed with wrong worker role!");

			//remove item from worker
			(*it)->BuildFinished();

			//reassign worker to mining
			NewWorker((*it)->GetPointer());
			_builders.erase(it);
			return;
		}
	}
	Log::Instance()->Assert(false,"Item in building state without worker!"); //item was in building state but no worker was building it
}

int WorkersModule::WorkerCountMinerals()
{
	int result = 0;
	for (const auto& exp : _expansionList)
		result += exp->WorkerCountMinerals();
	return result;
}

int WorkersModule::WorkerCountGas()
{
	int result = 0;
	for (const auto& exp : _expansionList)
		result += exp->WorkerCountGas();
	return result;
}

int WorkersModule::WorkerCountAll()
{
	return WorkerCountMinerals() + WorkerCountGas() + _builders.size() + _repairers.size();
}

bool WorkersModule::ExpansionNeeded()
{
	for (auto& exp : _expansionList)
	{
		if (!exp->IsSaturated())
			return false;
	}
	return true;
}

bool WorkersModule::BasesFull()
{
	for (auto& exp : _expansionList)
	{
		if (!exp->IsFull())
			return false;
	}
	return true;
}

int WorkersModule::ExpansionCount()
{
	return _expansionList.size();
}

int WorkersModule::RefineryCount()
{
	int result = 0;
	for (auto& exp : _expansionList)
	{
		if (exp->GetRefinery())
			result++;
	}
	return result;
}

void WorkersModule::WorkerDefence(size_t size)
{
	std::vector<std::shared_ptr<KasoBot::Worker>> toMove;

	int keep = (WorkerCountAll() + ArmyModule::Instance()->WorkerArmy()->Workers().size()) / 2; //TODO configure how many workers to always keep mining

	if (keep < 3) keep = 3;

	for (auto& exp : _expansionList) //select workers
	{
		for (auto& worker : exp->Workers())
		{
			if (keep > 0) //keep few workers mining always
			{
				keep--;
				continue;
			}
			toMove.emplace_back(worker);
			if (toMove.size() >= size)
				break;
		}
		if (toMove.size() >= size)
			break;
	}

	for (auto& worker : toMove) //add selected to army
	{
		ArmyModule::Instance()->AddWorker(worker);
	}

	for (auto& worker : toMove) //remove from here
	{
		RemoveWorker(worker->GetPointer());
		worker->RemoveMineral();
	}

}

void WorkersModule::AskForWorkers()
{
	for (auto& exp : _expansionList)
	{
		AssignIdleWorkers(*exp);
	}
}
