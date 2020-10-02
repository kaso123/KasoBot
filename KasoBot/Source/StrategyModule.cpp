#include "StrategyModule.h"
#include "ProductionModule.h"
#include "WorkersModule.h"
#include "ScoutModule.h"
#include "Config.h"

#include "EnemyStrategy.h"
#include "OwnStrategy.h"
#include "Opener.h"
#include <time.h>

using namespace KasoBot;

StrategyModule* StrategyModule::_instance = 0;

StrategyModule::StrategyModule()
	: _enemyLostMinerals(0), _enemyLostGas(0), _activeOpener(nullptr), _activeOpenerName("random"), _activeEnemyStrat(nullptr)
	, _activeStrat(nullptr), _activeStratName("random")
{
	_productionCycle.clear();
}

StrategyModule::~StrategyModule()
{
	delete(_instance);
}

bool StrategyModule::MacroSaturation()
{
	//TODO check if refinery is needed

	//check if we need to expand
	if (WorkersModule::Instance()->ExpansionNeeded())
	{
		if (!ProductionModule::Instance()->IsInQueue(BWAPI::UnitTypes::Terran_Command_Center))
			return ProductionModule::Instance()->BuildBuilding(BWAPI::UnitTypes::Terran_Command_Center);
	}

	if (WorkersModule::Instance()->WorkerCountMinerals() + WorkersModule::Instance()->WorkerCountGas() >= Config::Workers::MaxGlobal())
		return false;

	if (WorkersModule::Instance()->BasesFull())
		return false;

	return ProductionModule::Instance()->BuildUnit(BWAPI::UnitTypes::Terran_SCV);
}

bool StrategyModule::MacroArmy()
{
	for (auto& type : _activeStrat->GetMacroArmyTypes())
	{
		if (ProductionModule::Instance()->BuildUnit(type))
			return true;
	}
	return false;
}

bool StrategyModule::MacroTech()
{
	auto macro = _activeStrat->GetMacroTechType();

	if (macro._unit != BWAPI::UnitTypes::None)
	{
		_ASSERT(macro._unit.isBuilding());
		return ProductionModule::Instance()->BuildBuilding(macro._unit);
	}
	if (macro._upgrade != BWAPI::UpgradeTypes::None)
	{
		return ProductionModule::Instance()->MakeTech(macro._upgrade);
	}
	if (macro._tech != BWAPI::TechTypes::None)
	{
		return ProductionModule::Instance()->MakeTech(macro._tech);
	}

	return false;
}

bool StrategyModule::MacroProduction()
{
	BWAPI::UnitType next = _activeStrat->GetMacroProductionType();
	if (next == BWAPI::UnitTypes::None)
		return false;

	if (!ProductionModule::Instance()->CheckResources(next) || ProductionModule::Instance()->IsInQueue(next))
		return false;

	return ProductionModule::Instance()->BuildBuilding(next);
}

StrategyModule* StrategyModule::Instance()
{
	if (!_instance)
		_instance = new StrategyModule;
	return _instance;
}

void StrategyModule::OnFrame()
{
	if (_activeOpener)
	{
		if (ProductionModule::Instance()->NewTask(_activeOpener->Next()))
		{
			//task successful
			if (_activeOpener->Pop())
			{
				//opener was finished
				_activeOpener = nullptr;
				_activeOpenerName = "finished";
			}
		}
		return;
	}

	//cycle through all macro stuff in order and try to build something
	for (auto& item : _productionCycle)
	{
		if (item == Production::Type::SATURATION)
		{
			if (MacroSaturation())
				break;
		}
		if (item == Production::Type::ARMY)
		{
			if (MacroArmy())
				break;
		}
		if (item == Production::Type::TECH)
		{
			if (MacroTech())
				break;
		}
		if (item == Production::Type::PRODUCTION)
		{
			if (MacroProduction())
				break;
		}			
	}


}

void StrategyModule::EnemyDestroyed(BWAPI::UnitType type)
{
	_enemyLostMinerals += type.mineralPrice();
	_enemyLostGas += type.gasPrice();
}

void StrategyModule::NewOpener(const std::string & name, nlohmann::json & array)
{
	_openers.emplace(name, std::make_unique<Opener>(array));
}

void StrategyModule::SetOpener(const std::string & name)
{
	_ASSERT(!_openers.empty());

	if (name == "random" || _openers.find(name) == _openers.end())
	{
		srand(unsigned int(time(NULL)));
		//choose random opener
		auto it = _openers.begin();
		std::advance(it, rand() % _openers.size());
		_activeOpener = it->second.get();
		_activeOpenerName = "random: "+it->first;
		return;
	}

	//set opener
	_activeOpenerName = name;
	_activeOpener = _openers[name].get();
}

void StrategyModule::SetStrategy(const std::string& name)
{
	_ASSERT(!_strategies.empty());

	if (name == "random" || _strategies.find(name) == _strategies.end())
	{
		srand(unsigned int(time(NULL)));
		//choose random strat
		auto it = _strategies.begin();
		std::advance(it, rand() % _strategies.size());
		_activeStrat = it->second.get();
		_activeStratName = "random: " + it->first;
		SetOpener(_activeStrat->GetOpener());
		return;
	}

	//set strat
	_activeStratName = name;
	_activeStrat = _strategies[name].get();
	SetOpener(_activeStrat->GetOpener());
}

void StrategyModule::SetCycle(nlohmann::json & itemsArray)
{
	_ASSERT(itemsArray.is_array());

	_productionCycle.clear();

	for (auto& item : itemsArray)
	{
		_ASSERT(item.is_string());
		if (item == "saturation")
			_productionCycle.emplace_back(Production::Type::SATURATION);
		else if (item == "army")
			_productionCycle.emplace_back(Production::Type::ARMY);
		else if (item == "tech")
			_productionCycle.emplace_back(Production::Type::TECH);
		else if (item == "production")
			_productionCycle.emplace_back(Production::Type::PRODUCTION);

	}
}

void StrategyModule::NewEnemyStrategy(BWAPI::Race race, nlohmann::json & strat, int id)
{
	_ASSERT(strat.contains("name"));
	_ASSERT(strat["name"].is_string());

	EnemyStrategy* strategy = nullptr;
	if (race == BWAPI::Races::Terran)
		strategy = _stratsT.emplace_back(std::make_unique<EnemyStrategy>(strat["name"].get<std::string>(), id)).get();
	else if (race == BWAPI::Races::Protoss)
		strategy = _stratsP.emplace_back(std::make_unique<EnemyStrategy>(strat["name"].get<std::string>(), id)).get();
	else if (race == BWAPI::Races::Zerg)
		strategy = _stratsZ.emplace_back(std::make_unique<EnemyStrategy>(strat["name"].get<std::string>(), id)).get();
	else return;

	for (auto& item : strat["types"])
	{
		_ASSERT(item.contains("type"));
		_ASSERT(item["type"].is_string());
		_ASSERT(item.contains("value"));
		_ASSERT(item["value"].is_number_integer());
		_ASSERT(item.contains("include"));
		_ASSERT(item["include"].is_boolean());

		strategy->AddItem(Config::Utils::TypeFromString(item["type"]), item["value"].get<int>(),
			item.contains("limit") ? item["limit"].get<int>() : 1, item["include"].get<bool>());
	}
}

void StrategyModule::NewOwnStrategy(nlohmann::json& strat)
{
	_ASSERT(strat.contains("name"));
	_ASSERT(strat["name"].is_string());
	_ASSERT(strat.contains("opener"));
	_ASSERT(strat["opener"].is_string());

	OwnStrategy* strategy = _strategies.emplace(strat["name"].get<std::string>(), 
		std::make_unique<OwnStrategy>(strat["name"].get<std::string>(), strat["opener"].get<std::string>())).first->second.get();
	
	if (strat.contains("units"))
	{
		_ASSERT(strat["units"].is_object());

		for (auto it = strat["units"].begin(); it != strat["units"].end(); it++)
		{
			strategy->AddUnit(Config::Utils::TypeFromString(it.key()), it.value());
		}
	}
	if (strat.contains("tech"))
	{
		_ASSERT(strat["tech"].is_array());

		for (auto& item : strat["tech"])
		{
			strategy->AddTech(Config::Utils::TechTypeFromString(item));
		}
	}
	strategy->CalculateProduction();
}

const std::vector<std::unique_ptr<EnemyStrategy>>& StrategyModule::GetEnemyStrategies()
{
	if (ScoutModule::Instance()->GetEnemyRace() == BWAPI::Races::Terran)
		return _stratsT;
	if (ScoutModule::Instance()->GetEnemyRace() == BWAPI::Races::Protoss)
		return _stratsP;

	return _stratsZ;
}

