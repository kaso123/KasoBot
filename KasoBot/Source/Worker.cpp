#include "Worker.h"
#include "MapModule.h"
#include "BehaviourWorker.h"
#include "ProductionItem.h"
#include "Task.h"
#include "Army.h"

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
		_ASSERT(_mineral->Data() >= 0);
 		_mineral = nullptr;
	}

	if (_item) //set appropriate state to productionItem
	{
		_ASSERT(_item->GetState() == Production::State::ASSIGNED || _item->GetState() == Production::State::BUILDING);
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
		_ASSERT(_mineral->Data() >= 0);
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
		_ASSERT(_mineral->Data() >= 0);
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
		_ASSERT(_mineral->Data() >= 0);
	}
	_refinery = nullptr;
	_mineral = nullptr;
	

	_ASSERT(item->GetState() == Production::State::WAITING || item->GetState() == Production::State::UNFINISHED);
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
	_ASSERT(_workerRole == Workers::Role::BUILD || _workerRole == Workers::Role::ASSIGNED);
	_ASSERT(_item);
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

	_ASSERT(_behaviour);
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
	_ASSERT(_mineral->Data() >= 0);

	_mineral = nullptr;
	return true;
}
