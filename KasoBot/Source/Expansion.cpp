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

void Expansion::RemoveWorker(BWAPI::Unit unit)
{

}
