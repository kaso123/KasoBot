#include "Expansion.h"
#include "MapModule.h"
#include "Worker.h"

using namespace KasoBot;

Expansion::Expansion(BWAPI::Unit unit)
	:_pointer(unit), _station(nullptr)
{
	//find which station this expansion belongs to
	_station = KasoBot::Map::GetStation(unit->getTilePosition());

	_ASSERT(_station);
}

Expansion::~Expansion()
{
	//TODO transfer workers to another expansion / army
}

void Expansion::AddWorker(BWAPI::Unit unit)
{
	_workerList.emplace_back(std::make_shared<Worker>(unit));
}

void Expansion::AddWorker(std::shared_ptr<Worker> worker)
{
	_workerList.emplace_back(worker);
}

bool Expansion::RemoveWorker(BWAPI::Unit unit)
{
	int before = _workerList.size();

	_workerList.erase(std::remove_if(_workerList.begin(), _workerList.end(),
		[unit](auto& x)
		{
			return unit == x->GetPointer();
		}
	), _workerList.end());

	//check if worker was removed from list
	return before > _workerList.size();
}

bool Expansion::IsSaturated()
{
	//TODO implement IsSaturated
	return false;
}

bool Expansion::IsFull()
{
	//TODO implement IsFull
	return false;
}
