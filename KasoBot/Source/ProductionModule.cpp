#include "ProductionModule.h"
#include "WorkersModule.h"
#include "ArmyModule.h"
#include "MapModule.h"
#include "Config.h"
#include "StrategyModule.h"
#include "ScoutModule.h"

#include "ProductionItem.h"
#include "Unit.h"
#include "Expansion.h"
#include "Army.h"
#include "EnemyArmy.h"

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

void ProductionModule::PreventSupplyBlock()
{	
	//check how many supply depots are ordered already
	int suppliesInQueue = 0;
	for (auto& item : _items)
	{
		if (item->GetState() == Production::State::DONE)
			continue;

		if (item->GetType() == BWAPI::UnitTypes::Terran_Supply_Depot)
			suppliesInQueue++;
	}
	if (BWAPI::Broodwar->self()->supplyTotal() + suppliesInQueue * 16 >= 400)
		return;

	if (BWAPI::Broodwar->self()->supplyTotal() + suppliesInQueue * 16 < (int)std::floor(BWAPI::Broodwar->self()->supplyUsed() * Config::Production::FreeSupplyMultiplier()))
	{
		BuildBuilding(BWAPI::UnitTypes::Terran_Supply_Depot);
	}
}

bool ProductionModule::IsSafeToBuild(BWAPI::TilePosition pos)
{
	if (!pos.isValid())
		return true;

	for (auto& army : ScoutModule::Instance()->GetArmies())
	{
		if (army->BoundingBox()._center.getDistance(BWAPI::Position(pos)) < 500)
			return false;
	}
	return true;
}

ProductionModule* ProductionModule::Instance()
{
	if (!_instance)
		_instance = new ProductionModule;
	return _instance;
}

void ProductionModule::OnFrame()
{
	if(!StrategyModule::Instance()->IsOpenerActive() || BWAPI::Broodwar->self()->supplyTotal() <= BWAPI::Broodwar->self()->supplyUsed())
		PreventSupplyBlock();

	//sort items by status
	std::sort(_items.begin(), _items.end(),
		[](const std::unique_ptr<ProductionItem>& a, const std::unique_ptr<ProductionItem>& b) {return a->GetState() < b->GetState(); });

	//remove finished  items
	_items.erase(std::remove_if(_items.begin(), _items.end(),
		[](auto& x)
		{
			if (x->GetState() == Production::State::DONE)
				return true;
			return false;
		}
	), _items.end());


	for (auto& item : _items)
	{
		if (item->GetState() == Production::State::UNFINISHED)
		{
			if(IsSafeToBuild(item->GetLocation()))
				WorkersModule::Instance()->Build(item.get());
			return;
		}
		if (item->GetState() == Production::State::WAITING)
		{
			if (CanSendWorker(item->GetType()) && IsSafeToBuild(item->GetLocation()))
			{
				WorkersModule::Instance()->Build(item.get());
			}
			return;
		}
	}
}

void ProductionModule::AddUnit(BWAPI::Unit unit)
{
	auto it = _unitList.find(unit->getType());
	
	if (it != _unitList.end())
	{
		ArmyModule::Instance()->AddSoldier(it->second.emplace_back(std::make_unique<KasoBot::Unit>(unit)).get());
	}
	else
	{
		auto new_it = _unitList.insert({ unit->getType(), UnitList{} });
		ArmyModule::Instance()->AddSoldier(new_it.first->second.emplace_back(std::make_unique<KasoBot::Unit>(unit)).get());
	}
}

void ProductionModule::AddBuilding(BWAPI::Unit unit)
{
	auto it = _buildingList.find(unit->getType());

	KasoBot::Unit* created = nullptr;
	if (it != _buildingList.end())
	{
		created = it->second.emplace_back(std::make_unique<KasoBot::Unit>(unit)).get();
	}
	else
	{
		auto new_it = _buildingList.insert({ unit->getType(), UnitList{} });
		created = new_it.first->second.emplace_back(std::make_unique<KasoBot::Unit>(unit)).get();
	}

	_ASSERT(created);

	if (unit->getType() == BWAPI::UnitTypes::Terran_Bunker)
	{
		if (!ArmyModule::Instance()->Bunker())
		{
			ArmyModule::Instance()->SetBunker(created);
		}
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

	//remove bunker if destroyed
	if (unit->getType() == BWAPI::UnitTypes::Terran_Bunker)
	{
		if (ArmyModule::Instance()->Bunker() 
			&& ArmyModule::Instance()->Bunker()->GetPointer() == unit)
		{
			ArmyModule::Instance()->SetBunker(nullptr);
			BuildBuilding(BWAPI::UnitTypes::Terran_Bunker);
		}
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
		if (building->IsLocked()) //only one locked building of one type at each type
		{ 
			if (building->GetPointer()->isIdle())
			{
				if (building->GetPointer()->buildAddon(type)) //build started
				{
					building->Unlock();
					return true;
				}
			}
			return false;
		}	
	}

	for (auto& building : (*it).second)
	{
		if (!building->GetPointer()->getAddon() && !building->GetPointer()->isConstructing() && !building->IsLocked())
		{
			building->Lock();
			return false;
		}
	}

	return false;
}

bool ProductionModule::BuildBuilding(BWAPI::UnitType type)
{
	_ASSERT(type.isBuilding());

	if (type.isAddon())
		return BuildAddon(type);

	if (_items.emplace_back(std::make_unique<ProductionItem>(type, KasoBot::Map::GetBuildPosition(type))))
		return true;

	return false;
}

bool ProductionModule::BuildRefineryAtExpansion(Expansion& exp)
{
	auto pos = BWAPI::Broodwar->getBuildLocation(BWAPI::UnitTypes::Terran_Refinery, exp.GetStation()->getBWEMBase()->Location());

	if(!pos.isValid())
		return false;

	if (_items.emplace_back(std::make_unique<ProductionItem>(BWAPI::UnitTypes::Terran_Refinery, pos)))
		return true;

	return false;

}

std::pair<bool,bool> ProductionModule::BuildUnit(BWAPI::UnitType type)
{
	_ASSERT(!type.isBuilding());

	if (type.isWorker())
		return { WorkersModule::Instance()->BuildWorker(), false };

	//find building type that builds this
	auto it = _buildingList.find(type.whatBuilds().first);
	if (it == _buildingList.end())
		return { false,false };

	for (auto& building : (*it).second)
	{
		if (building->GetPointer()->isIdle() && !building->IsLocked())
		{
			if (!CheckResources(type))
				return { false, true };

			if (building->GetPointer()->train(type)) //training started
			{
				return { true,false };
			}
		}
	}
	return { false, false };
}

bool ProductionModule::MakeTech(BWAPI::UpgradeType type)
{
	if (!CheckResources(type))
		return false;


	//find building type that builds this
	auto it = _buildingList.find(type.whatUpgrades());
	if (it == _buildingList.end())
		return false;

	for (auto& building : (*it).second)
	{
		if (building->GetPointer()->isIdle())
		{
			if (building->GetPointer()->upgrade(type)) //research started
			{
				return true;
			}
		}
	}
	return false;
}

bool ProductionModule::MakeTech(BWAPI::TechType type)
{
	if (!CheckResources(type))
		return false;

	//find building type that builds this
	auto it = _buildingList.find(type.whatResearches());
	if (it == _buildingList.end())
		return false;

	for (auto& building : (*it).second)
	{
		if (building->GetPointer()->isIdle())
		{
			if (building->GetPointer()->research(type)) //research started
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
	if ((type.mineralPrice() > 0) && (BWAPI::Broodwar->self()->minerals() - _reservedMinerals < type.mineralPrice()))
		return false;
	if ((type.gasPrice() > 0) && (BWAPI::Broodwar->self()->gas() - _reservedGas < type.gasPrice()))
		return false;

	return true;
}

bool ProductionModule::CheckResources(BWAPI::UpgradeType type)
{
	//consider level of upgrade also
	if (BWAPI::Broodwar->self()->minerals() - _reservedMinerals < type.mineralPrice(BWAPI::Broodwar->self()->getUpgradeLevel(type) + 1))
		return false;
	if (BWAPI::Broodwar->self()->gas() - _reservedGas < type.gasPrice(BWAPI::Broodwar->self()->getUpgradeLevel(type) + 1))
		return false;

	return true;
}

bool ProductionModule::CheckResources(BWAPI::TechType type)
{
	if (BWAPI::Broodwar->self()->minerals() - _reservedMinerals < type.mineralPrice())
		return false;
	if (BWAPI::Broodwar->self()->gas() - _reservedGas < type.gasPrice())
		return false;

	return true;
}

bool ProductionModule::CanSendWorker(BWAPI::UnitType type)
{
	int minQueued = 0;
	int gasQueued = 0;

	for (auto& item : _items)
	{
		if (item->GetState() == Production::State::ASSIGNED)
		{
			minQueued += item->GetType().mineralPrice();
			gasQueued += item->GetType().gasPrice();
		}
	}

	if (BWAPI::Broodwar->self()->minerals() - minQueued + WorkersModule::Instance()->WorkerCountMinerals() * Config::Workers::WorkerResourceValue()
		* (type.isResourceDepot() ? 2 : 1) < type.mineralPrice()) //double the amount for CCs, worker has to move longer
		return false;
	if (BWAPI::Broodwar->self()->gas() - gasQueued + WorkersModule::Instance()->WorkerCountGas() * Config::Workers::WorkerResourceValue() < type.gasPrice())
		return false;

	//check if all needed buildings are finished
	for (auto& req : type.requiredUnits())
	{
		if (!BWAPI::Broodwar->self()->hasUnitTypeRequirement(req.first, req.second))
			return false;
	}
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

	return BuildUnit(type).first;
}

bool ProductionModule::IsInQueue(BWAPI::UnitType type)
{
	for (const auto& item : _items)
	{
		if (item->GetState() == Production::State::DONE)
			continue;

		if (item->GetType() == type)
			return true;
	}
	return false;
}

int ProductionModule::GetCountOf(BWAPI::UnitType type)
{
	if (type.isBuilding())
	{
		int inProgress = 0;

		for (auto& item : _items)
		{
			if (item->GetState() != Production::State::DONE && item->GetType() == type)
				inProgress++;
		}

		if (type == BWAPI::UnitTypes::Terran_Refinery)
			return inProgress + WorkersModule::Instance()->RefineryCount();
		if (type == BWAPI::UnitTypes::Terran_Command_Center)
			return inProgress + WorkersModule::Instance()->ExpansionCount();

		auto it = _buildingList.find(type);

		if (it == _buildingList.end())
			return inProgress;

		return it->second.size() + inProgress;
	}
	else
	{
		if (type.isWorker())
			return WorkersModule::Instance()->WorkerCountMinerals() + WorkersModule::Instance()->WorkerCountGas();

		auto it = _unitList.find(type);

		if (it == _unitList.end())
			return 0;
		return it->second.size();
	}
	return 0;
}

void ProductionModule::TileOccupied(BWAPI::Unit unit)
{
	for (auto& item : _items)
	{
		if (item->GetState() == Production::State::DONE
			|| item->GetState() == Production::State::BUILDING)
			continue;

		if (!item->GetLocation().isValid())
			continue;

		//check if this unit is in a way of building our build item
		if (unit->getTilePosition().x + unit->getType().tileWidth() <= item->GetLocation().x)
			continue;
		if (unit->getTilePosition().y + unit->getType().tileHeight() <= item->GetLocation().y)
			continue;
		if (unit->getTilePosition().x > item->GetLocation().x + item->GetType().tileWidth())
			continue;
		if (unit->getTilePosition().y > item->GetLocation().y + item->GetType().tileHeight())
			continue;
		
		//item's build position is occupied by discovered building
		BWEB::Map::KasoBot::UnreserveTiles(item->GetLocation(), item->GetType());
		item->SetLocation(BWAPI::TilePositions::Invalid);
	}
}
