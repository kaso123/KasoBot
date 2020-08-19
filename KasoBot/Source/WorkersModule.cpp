#include "WorkersModule.h"
#include "ArmyModule.h"

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

std::shared_ptr<Expansion> WorkersModule::FindExpansionForWorker(BWAPI::Unit unit)
{
	std::shared_ptr<Expansion> closestFree = nullptr;
	std::shared_ptr<Expansion> closest = nullptr;
	int distFree = INT_MAX;
	int dist = INT_MAX;

	for (const auto exp : _expansionList)
	{
		int currDist = unit->getDistance(exp->GetPointer());
		if (currDist < dist && !exp->IsFull())
		{
			closest = exp;
			dist = unit->getDistance(exp->GetPointer());
		}
		if (currDist < distFree && !exp->IsSaturated())
		{
			closestFree = exp;
			distFree = unit->getDistance(exp->GetPointer());
		}
	}

	//try to find not saturated base first, then not oversaturated base
	if(closestFree)
		return closestFree;
	
	return closest;
}

void WorkersModule::AssignIdleWorkers(std::shared_ptr<Expansion> exp)
{
	//get workers from army
	auto list = ArmyModule::Instance()->GetFreeWorkers(INT_MAX); //TODO add real limit

	for (auto worker : list)
	{
		exp->AddWorker(worker);
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
	std::shared_ptr<Expansion> closest = FindExpansionForWorker(unit);
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
}

void WorkersModule::ExpansionCreated(BWAPI::Unit unit)
{
	//create new expansion object and assign workers from army or oversaturated bases
	AssignIdleWorkers(_expansionList.emplace_back(std::make_shared<Expansion>(unit)));
}

void WorkersModule::ExpansionDestroyed(BWAPI::Unit unit)
{
}
