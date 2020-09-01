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

	//if not close to position -> move there
	if(worker.GetPointer()->getDistance(Map::GetCenterOfBuilding(worker.GetProductionItem()->GetLocation(), worker.GetProductionItem()->GetType())) > Config::Workers::BuildStartDistance())
	{
		Move(worker.GetPointer(), BWAPI::Position(worker.GetProductionItem()->GetLocation()));
		return;
	}
	else //start building
	{
		//if started switch Role and update ProductionItem
		if (worker.GetPointer()->isConstructing() && worker.GetPointer()->getBuildUnit())
		{
			_ASSERT(worker.GetPointer()->getBuildType() == worker.GetProductionItem()->GetType());

			worker.SetWorkerRole(Workers::Role::BUILD);
			worker.GetProductionItem()->BuildStarted();
			return;
		}

		Build(worker.GetPointer(),worker.GetProductionItem()->GetLocation(),worker.GetProductionItem()->GetType());
		return;
	}

	
}

void BehaviourWorker::Construct(Worker& worker)
{
	_ASSERT(worker.GetProductionItem());
	

	if (worker.GetPointer()->isConstructing())
	{
		_ASSERT(worker.GetProductionItem()->GetState() == Production::State::BUILDING);
		return;
	}
		
	//if finished -> change state, switch to minerals
	if (worker.GetProductionItem()->GetState() == Production::State::DONE)
	{
		//worker should be reassigned inside WorkersModule
		return;
	}
		
	//not finished, reset to assigned
	worker.SetWorkerRole(Workers::Role::ASSIGNED);
	worker.GetProductionItem()->Restart();
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

void BehaviourWorker::Build(BWAPI::Unit unit, BWAPI::TilePosition pos, BWAPI::UnitType type)
{
	if ((unit->getOrder() == BWAPI::Orders::ConstructingBuilding
		&& unit->getBuildType() == type)
		||
		(unit->getOrder() == BWAPI::Orders::PlaceBuilding 
		&& unit->getOrderTargetPosition() == (BWAPI::Position)pos))
		return;

	unit->build(type, pos);
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
