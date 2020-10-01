#include "OwnStrategy.h"
#include "StrategyModule.h"

using namespace KasoBot;

OwnStrategy::OwnStrategy(std::string& name, std::string& opener)
	:_name(name), _opener(opener)
{
}

OwnStrategy::~OwnStrategy()
{
}

void OwnStrategy::AddUnit(BWAPI::UnitType type, int count)
{
	_units.emplace_back(std::make_pair(type, count));
}

void OwnStrategy::AddTech(Production::TechMacro macro)
{
	_tech.emplace_back(macro);
}
