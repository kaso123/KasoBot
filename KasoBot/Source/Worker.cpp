#include "Worker.h"
#include "MapModule.h"
#include "BehaviourWorker.h"
#include "ProductionItem.h"
#include "Task.h"
#include "Army.h"
#include "Log.h"

using namespace KasoBot;

Worker::Worker(BWAPI::Unit unit)
	: Unit::Unit(unit), _workerRole(Workers::Role::IDLE)
	, _mineral(nullptr), _refinery(nullptr), _item(nullptr)
{
}

Worker::~Worker()
{
	if (_mineral) //remove self from mineral
	{
		_mineral->SetData(_mineral->Data() - 1);
		Log::Instance()->Assert(_mineral->Data() >= 0,"Negative worker count on mineral - destructor!");
 		_mineral = nullptr;
	}

	if (_item) //set appropriate state to productionItem
	{
		Log::Instance()->Assert(_item->GetState() == Production::State::ASSIGNED || _item->GetState() == Production::State::BUILDING,"Wrong item state in worker desctructor!");
		_item->WorkerDied();
		_item = nullptr;
	}
}

void Worker::AssignRoleMinerals(BWEM::Mineral* mineral)
{
	if (_mineral)
	{
		//decrease number of workers for previous mineral
		_mineral->SetData(_mineral->Data() - 1);
		Log::Instance()->Assert(_mineral->Data() >= 0,"Negative worker count on mineral - assigning!");
	}

	_refinery = nullptr;
	_mineral = mineral;
	_workerRole = Workers::Role::MINERALS;

	//increase number of workers for mineral
	_mineral->SetData(_mineral->Data() + 1);
}

void Worker::AssignRoleGas(BWAPI::Unit refinery)
{
	if (_mineral)
	{
		//decrease number of workers for mineral
		_mineral->SetData(_mineral->Data() - 1);
		Log::Instance()->Assert(_mineral->Data() >= 0,"Negative worker count on mineral - assigning!");
	}
	_mineral = nullptr;
	_refinery = refinery;
	_workerRole = Workers::Role::GAS;
}

bool Worker::AssignRoleBuild(ProductionItem* item)
{
	if (_item)
		return false;

	if (_mineral)
	{
		//decrease number of workers for mineral
		_mineral->SetData(_mineral->Data() - 1);
		Log::Instance()->Assert(_mineral->Data() >= 0, "Negative worker count on mineral - assign build!");
	}
	_refinery = nullptr;
	_mineral = nullptr;
	

	Log::Instance()->Assert(item->GetState() == Production::State::WAITING || item->GetState() == Production::State::UNFINISHED,"Wrong item state when assigning to worker!");
	item->Assigned();
	_item = item;
	_workerRole = Workers::Role::ASSIGNED;
	return true;
}

bool Worker::IsMiningMineral(BWAPI::Unit mineral)
{
	if (!_mineral)
		return false;

	if (mineral == _mineral->Unit())
		return true;

	return false;
}

void Worker::BuildFinished()
{
	Log::Instance()->Assert(_workerRole == Workers::Role::BUILD || _workerRole == Workers::Role::ASSIGNED,"Wrong item state when finishing!");
	Log::Instance()->Assert(_item, "Fininshed worker has no item!");
	_item = nullptr;
}

void Worker::Work()
{
	if (_playerControl)
		return;

	//set new build location if it was reset
	if (_item && _item->GetLocation() == BWAPI::TilePositions::Invalid)
	{
		_item->SetLocation(Map::GetBuildPosition(_item->GetType()));
		if(_item->GetLocation().isValid())
			BWEB::Map::KasoBot::ReserveTiles(_item->GetLocation(), _item->GetType());
		else return;
	}

	Log::Instance()->Assert(_behaviour.get(),"Worker has no behaviour!");
	_behaviour->Work(*this);
}

void Worker::Fight(Army* army)
{
	if (army->Task()->Type() == Tasks::Type::DEFEND)
	{
		_behaviour->DefendArmy(*this,army);
	}
}

bool Worker::RemoveMineral()
{
	if (!_mineral)
		return false;

	_mineral->SetData(_mineral->Data() - 1);
	Log::Instance()->Assert(_mineral->Data() >= 0,"Negative worker count on mineral - remove mineral!");

	_mineral = nullptr;
	_workerRole = Workers::Role::IDLE;
	return true;
}
