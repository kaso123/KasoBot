#include "Worker.h"
#include "MapModule.h"

using namespace KasoBot;

Worker::Worker(BWAPI::Unit unit)
	: Unit::Unit(unit), _workerRole(Workers::Role::IDLE), _mineral(nullptr), _refinery(nullptr)
{
}

Worker::~Worker()
{
}

void Worker::AssignRoleMinerals(BWEM::Mineral* mineral)
{
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
