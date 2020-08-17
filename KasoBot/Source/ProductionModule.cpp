#include "ProductionModule.h"
#include "Unit.h"

using namespace KasoBot;

ProductionModule* ProductionModule::_instance = 0;

ProductionModule::ProductionModule()
{
}

ProductionModule::~ProductionModule()
{
	delete(_instance);
}

ProductionModule* ProductionModule::Instance()
{
	if (!_instance)
		_instance = new ProductionModule;
	return _instance;
}

void KasoBot::ProductionModule::AddUnit(BWAPI::Unit unit)
{
	auto it = _unitList.find(unit->getType());
	
	if (it != _unitList.end())
	{
		it->second.emplace_back(std::make_shared<KasoBot::Unit>(unit));
	}
	else
	{
		auto new_it = _unitList.insert({ unit->getType(), UnitList{} });
		new_it.first->second.emplace_back(std::make_shared<KasoBot::Unit>(unit));
	}
}

void KasoBot::ProductionModule::AddBuilding(BWAPI::Unit unit)
{
	auto it = _buildingList.find(unit->getType());

	if (it != _buildingList.end())
	{
		it->second.emplace_back(std::make_shared<KasoBot::Unit>(unit));
	}
	else
	{
		auto new_it = _buildingList.insert({ unit->getType(), UnitList{} });
		new_it.first->second.emplace_back(std::make_shared<KasoBot::Unit>(unit));
	}
}
