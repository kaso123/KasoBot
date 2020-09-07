#include "ProductionModule.h"
#include "WorkersModule.h"
#include "MapModule.h"

#include "ProductionItem.h"
#include "Unit.h"

using namespace KasoBot;

ProductionModule* ProductionModule::_instance = 0;

ProductionModule::ProductionModule()
	: _reservedMinerals(0), _reservedGas(0)
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

void ProductionModule::OnFrame()
{
	for (auto& item : _items)
	{
		if (item->GetState() == Production::State::WAITING || item->GetState() == Production::State::UNFINISHED)
		{
			WorkersModule::Instance()->Build(item.get());
			return;
		}		
	}
}

void ProductionModule::AddUnit(BWAPI::Unit unit)
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

void ProductionModule::AddBuilding(BWAPI::Unit unit)
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

void ProductionModule::RemoveUnit(BWAPI::Unit unit)
{
	if (!unit->isCompleted())
		return;

	auto it = _unitList.find(unit->getType());

	_ASSERT(it != _unitList.end());
	
	//find unit in list and erase it
	it->second.erase(std::remove_if(it->second.begin(), it->second.end(),
		[unit](auto& x)
		{
			return unit == x->GetPointer();
		}
	),it->second.end());

	//if the list for this type is empty now, remove the list
	if (it->second.empty())
	{
		_unitList.erase(it);
	}
}

void ProductionModule::RemoveBuilding(BWAPI::Unit unit)
{
	if (!unit->isCompleted())
	{
		//find building in ProductionItems and change its status
		for (auto& item : _items)
		{
			if (unit->getTilePosition() == item->GetLocation())
			{
				_ASSERT(unit->getType() == item->GetType());
				item->BuildingDestroyed();
				return;
			}
		}
		_ASSERT(false); //there can't be uncomplete building without a production item
	}

	auto it = _buildingList.find(unit->getType());

	_ASSERT(it != _buildingList.end());

	//find unit in list and erase it
	it->second.erase(std::remove_if(it->second.begin(), it->second.end(),
		[unit](auto& x)
		{
			return unit == x->GetPointer();
		}
	), it->second.end());

	//if the list for this type is empty now, remove the list
	if (it->second.empty())
	{
		_buildingList.erase(it);
	}
}

bool ProductionModule::BuildAddon(BWAPI::UnitType type)
{
	_ASSERT(type.isAddon());

	if (!CheckResources(type))
		return false;

	if (type.whatBuilds().first == BWAPI::UnitTypes::Terran_Command_Center)
		return WorkersModule::Instance()->BuildAddon(type);

	//find building type that builds this
	auto it = _buildingList.find(type.whatBuilds().first);
	if (it == _buildingList.end())
		return false;

	for (auto& building : (*it).second)
	{
		if (building->GetPointer()->isIdle())
		{ 
			if (building->GetPointer()->buildAddon(type)) //build started
			{
				return true;
			}
		}	
	}
	return false;
}

bool ProductionModule::BuildBuilding(BWAPI::UnitType type)
{
	_ASSERT(type.isBuilding() && !type.isAddon());

	if (_items.emplace_back(std::make_unique<ProductionItem>(type, KasoBot::Map::GetBuildPosition(type))))
		return true;

	return false;
}

bool ProductionModule::BuildUnit(BWAPI::UnitType type)
{
	_ASSERT(!type.isBuilding());

	if (!CheckResources(type))
		return false;

	if (type.isWorker())
		return WorkersModule::Instance()->BuildWorker();

	//find building type that builds this
	auto it = _buildingList.find(type.whatBuilds().first);
	if (it == _buildingList.end())
		return false;

	for (auto& building : (*it).second)
	{
		if (building->GetPointer()->isIdle())
		{
			if (building->GetPointer()->train(type)) //training started
			{
				return true;
			}
		}
	}
	return false;
}

void ProductionModule::DebugBuild(BWAPI::UnitType type)
{
	if (type.isAddon())
	{
		BuildAddon(type);
		return;
	}		
	BWAPI::TilePosition buildPos = KasoBot::Map::GetBuildPosition(type);

	WorkersModule::Instance()->Build(_items.emplace_back(std::make_unique<ProductionItem>(type, buildPos)).get());
}

void ProductionModule::ReserveResources(BWAPI::UnitType type)
{
	_reservedMinerals += type.mineralPrice();
	_reservedGas += type.gasPrice();
}

void ProductionModule::FreeResources(BWAPI::UnitType type)
{
	_reservedMinerals -= type.mineralPrice();
	_reservedGas -= type.gasPrice();

	_ASSERT(_reservedMinerals >= 0);
	_ASSERT(_reservedGas >= 0);
}

bool ProductionModule::CheckResources(BWAPI::UnitType type)
{
	if (BWAPI::Broodwar->self()->minerals() - _reservedMinerals < type.mineralPrice())
		return false;
	if (BWAPI::Broodwar->self()->gas() - _reservedGas < type.gasPrice())
		return false;

	return true;
}

bool ProductionModule::NewTask(BWAPI::UnitType type)
{
	if (type.isBuilding())
	{
		if (type.isAddon())
			return BuildAddon(type);

		return BuildBuilding(type);
	}

	return BuildUnit(type);
}
