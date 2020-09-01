#include "Worker.h"
#include "MapModule.h"
#include "BehaviourWorker.h"
#include "ProductionItem.h"

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
	

	_ASSERT(item->GetState() == Production::State::WAITING);
	item->Assigned();
	_item = item;
	_workerRole = Workers::Role::ASSIGNED;
	return true;
}

bool Worker::IsMiningMineral(BWAPI::Unit mineral)
{
	if (mineral == _mineral->Unit())
		return true;

	return false;
}

void Worker::Work()
{
	if (_playerControl)
		return;

	_ASSERT(_behaviour);
	_behaviour->Work(*this);
}
