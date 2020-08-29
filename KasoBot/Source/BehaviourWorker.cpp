#include "BehaviourWorker.h"
#include "Worker.h"
#include "MapModule.h"
#include "ProductionItem.h"

using namespace KasoBot;

void BehaviourWorker::Minerals(Worker& worker)
{
	_ASSERT(worker.GetMineral());

	//return cargo when switched from gas
	if (worker.GetPointer()->isCarryingGas())
	{
		ReturnCargo(worker.GetPointer());
		return;
	}	

	GatherMinerals(worker.GetPointer(), worker.GetMineral()->Unit());
}

void BehaviourWorker::Gas(Worker& worker)
{
	_ASSERT(worker.GetRefinery());

	//return cargo if switched from minerals
	if (worker.GetPointer()->isCarryingMinerals())
	{
		ReturnCargo(worker.GetPointer());
		return;
	}
	
	GatherGas(worker.GetPointer(), worker.GetRefinery());
}

void BehaviourWorker::MoveToBuild(Worker& worker)
{
	_ASSERT(worker.GetProductionItem());
	_ASSERT(worker.GetProductionItem()->GetState() == Production::State::ASSIGNED);

	//TODO if not close to position -> move there
	//TODO else start building
	//TODO if started switch Role and update ProductionItem
}

void BehaviourWorker::Construct(Worker& worker)
{
	_ASSERT(worker.GetProductionItem());
	_ASSERT(worker.GetProductionItem()->GetState() == Production::State::BUILDING);

	//TODO if not constructing
		//if finished -> change state, switch to minerals
		//if not -> reset to ASSIGNED
}

void BehaviourWorker::GatherMinerals(BWAPI::Unit unit, BWAPI::Unit target)
{
	//worker is mining his mineral - do nothing
	if (((unit->getOrder() == BWAPI::Orders::MiningMinerals
		|| unit->getOrder() == BWAPI::Orders::WaitForMinerals
		|| unit->getOrder() == BWAPI::Orders::MoveToMinerals)
		&& unit->getOrderTarget() == target)
		|| unit->getOrder() == BWAPI::Orders::ReturnMinerals)
		return;

	unit->gather(target);
}

void BehaviourWorker::GatherGas(BWAPI::Unit unit, BWAPI::Unit target)
{
	//worker is mining gas from his refinery - do nothing
	if (((unit->getOrder() == BWAPI::Orders::HarvestGas
		|| unit->getOrder() == BWAPI::Orders::WaitForGas
		|| unit->getOrder() == BWAPI::Orders::MoveToGas
		|| unit->getOrder() == BWAPI::Orders::Harvest1)
		&& unit->getOrderTarget() == target)
		|| unit->getOrder() == BWAPI::Orders::ReturnGas)
		return;

	unit->gather(target);
}

void BehaviourWorker::ReturnCargo(BWAPI::Unit unit)
{
	if (unit->getOrder() == BWAPI::Orders::ReturnGas || unit->getOrder() == BWAPI::Orders::ReturnMinerals)
		return;

	unit->returnCargo();
}

BehaviourWorker::BehaviourWorker()
{
}

BehaviourWorker::~BehaviourWorker()
{
}

void BehaviourWorker::Work(Worker& worker)
{
	if (worker.GetPointer()->getLastCommandFrame() + Config::Units::OrderDelay() > BWAPI::Broodwar->getFrameCount())
		return;

	if (worker.GetWorkerRole() == Workers::Role::MINERALS)
	{
		Minerals(worker);
		return;
	}

	if (worker.GetWorkerRole() == Workers::Role::GAS)
	{
		Gas(worker);
		return;
	}

	if (worker.GetWorkerRole() == Workers::Role::ASSIGNED)
	{
		MoveToBuild(worker);
		return;
	}

	if (worker.GetWorkerRole() == Workers::Role::BUILD)
	{
		Construct(worker);
		return;
	}
		
}
