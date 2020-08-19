#include "Expansion.h"
#include "Worker.h"

using namespace KasoBot;

Expansion::Expansion(BWAPI::Unit unit)
{
	_pointer = unit;
}

Expansion::~Expansion()
{
}

void Expansion::AddWorker(BWAPI::Unit unit)
{
	_workerList.emplace_back(std::make_shared<Worker>(unit));
}

void Expansion::AddWorker(std::shared_ptr<Worker> worker)
{
	_workerList.emplace_back(worker);
}

void Expansion::RemoveWorker(BWAPI::Unit unit)
{

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
