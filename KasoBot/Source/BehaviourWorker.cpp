#include "BehaviourWorker.h"
#include "Worker.h"
#include "MapModule.h"
#include "ArmyModule.h"
#include "ProductionItem.h"
#include "Log.h"

using namespace KasoBot;

void BehaviourWorker::Minerals(Worker& worker)
{
	Log::Instance()->Assert(worker.GetMineral(),"No mineral for worker when mining!");

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
	Log::Instance()->Assert(worker.GetRefinery(),"No refinery for worker when mining gas!");

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
	Log::Instance()->Assert(worker.GetProductionItem(),"No production item in worker!");
	Log::Instance()->Assert(worker.GetProductionItem()->GetState() == Production::State::ASSIGNED, "Item state is not ASSIGNED!");

	//if not close to position -> move there
	if(worker.GetPointer()->getDistance(Map::GetCenterOfBuilding(worker.GetProductionItem()->GetLocation(), worker.GetProductionItem()->GetType())) > Config::Workers::BuildStartDistance())
	{
		Move(worker.GetPointer(), Map::GetCenterOfBuilding(worker.GetProductionItem()->GetLocation(), worker.GetProductionItem()->GetType()));
		return;
	}
	else //start building
	{
		//if started switch Role and update ProductionItem
		if (worker.GetPointer()->isConstructing() && worker.GetPointer()->getBuildUnit())
		{
			Log::Instance()->Assert(worker.GetPointer()->getBuildType() == worker.GetProductionItem()->GetType(),"Worker is building wrong type!");

			worker.SetWorkerRole(Workers::Role::BUILD);
			worker.GetProductionItem()->BuildStarted();
			return;
		}

		//unfinished building needs to be completed
		auto typeOnTile = BWEB::Map::isUsed(worker.GetProductionItem()->GetLocation(), worker.GetProductionItem()->GetType().tileWidth(), worker.GetProductionItem()->GetType().tileHeight());
		if ( typeOnTile != BWAPI::UnitTypes::None && typeOnTile != BWAPI::UnitTypes::Resource_Vespene_Geyser)
		{
			Log::Instance()->Assert(typeOnTile == worker.GetProductionItem()->GetType(),"Wrong unfinished type on build tile!");

			BWAPI::Unit unfinished = KasoBot::Map::GetUnfinished(worker.GetProductionItem()->GetLocation(), worker.GetProductionItem()->GetType());
			Log::Instance()->Assert(unfinished,"Unfinished building was not found!");

			Build(worker.GetPointer(), unfinished);
			return;
		}

		Build(worker.GetPointer(),worker.GetProductionItem()->GetLocation(),worker.GetProductionItem()->GetType());
		return;
	}
}

void BehaviourWorker::Construct(Worker& worker)
{
	Log::Instance()->Assert(worker.GetProductionItem(),"Worker doesn't have production item when constructing!");
	
	if (!worker.GetProductionItem())
		return;

	if (worker.GetPointer()->isConstructing())
	{
		Log::Instance()->Assert(worker.GetProductionItem()->GetState() == Production::State::BUILDING,"Worker is building sth else!");
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

void BehaviourWorker::Repair(BWAPI::Unit unit, BWAPI::Unit building)
{
	if (unit->getOrder() == BWAPI::Orders::Repair && unit->getOrderTarget() == building)
		return;

	unit->repair(building);
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

	if (!unit->build(type, pos))
	{
		if (BWAPI::Broodwar->getLastError() == BWAPI::Errors::None)
		{
			//move units away from build Position
			ArmyModule::Instance()->ClearTiles(pos, type);
		}
	}
}

void BehaviourWorker::Build(BWAPI::Unit unit, BWAPI::Unit building)
{
	if(unit->getOrder() == BWAPI::Orders::ConstructingBuilding && unit->getBuildUnit() == building)
		return;

	unit->rightClick(building);
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

void BehaviourWorker::DefendArmy(KasoBot::Unit & unit, Army * army)
{
	auto bunker = ArmyModule::Instance()->Bunker();
	if (unit.GetRole() == Units::Role::BUNKER && bunker
		&& bunker->GetPointer()->getHitPoints() < bunker->GetPointer()->getType().maxHitPoints())
	{
		Repair(unit.GetPointer(), ArmyModule::Instance()->Bunker()->GetPointer());
		return;
	}
	Behaviour::DefendArmy(unit,army);
}
