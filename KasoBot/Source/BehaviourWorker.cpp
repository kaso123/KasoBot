#include "BehaviourWorker.h"
#include "Worker.h"
#include "MapModule.h"

using namespace KasoBot;

void BehaviourWorker::Minerals(Worker& worker)
{
	_ASSERT(worker.GetMineral());

	//worker is mining his mineral - do nothing
	if (((worker.GetPointer()->getOrder() == BWAPI::Orders::MiningMinerals
		|| worker.GetPointer()->getOrder() == BWAPI::Orders::WaitForMinerals
		|| worker.GetPointer()->getOrder() == BWAPI::Orders::MoveToMinerals)
		&& worker.GetPointer()->getOrderTarget() == worker.GetMineral()->Unit())
		|| worker.GetPointer()->getOrder() == BWAPI::Orders::ReturnMinerals)
		return;

	Gather(worker.GetPointer(), worker.GetMineral()->Unit());
}

void BehaviourWorker::Gas(Worker& worker)
{
	_ASSERT(worker.GetRefinery());
	
	//TODO check orders for gas mining
	//Gather(worker.GetPointer(), worker.GetRefinery());
}

void BehaviourWorker::Gather(BWAPI::Unit unit, BWAPI::Unit target)
{
	unit->gather(target);
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
		
}
