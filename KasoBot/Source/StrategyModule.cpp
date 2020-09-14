#include "StrategyModule.h"
#include "ProductionModule.h"
#include "WorkersModule.h"
#include "Config.h"
#include "Opener.h"
#include <time.h>

using namespace KasoBot;

StrategyModule* StrategyModule::_instance = 0;

StrategyModule::StrategyModule()
	: _enemyLostMinerals(0), _enemyLostGas(0), _activeOpener(nullptr), _activeOpenerName("random")
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
	for (auto& type : GetMacroArmyTypes())
	{
		if (ProductionModule::Instance()->BuildUnit(type))
			return true;
	}
	return false;
}

bool StrategyModule::MacroTech()
{
	return false;
}

bool StrategyModule::MacroProduction()
{
	if (!ProductionModule::Instance()->CheckResources(GetMacroProductionType()) || ProductionModule::Instance()->IsInQueue(GetMacroProductionType()))
		return false;

	return ProductionModule::Instance()->BuildBuilding(GetMacroProductionType());
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

std::vector<BWAPI::UnitType> StrategyModule::GetMacroArmyTypes()
{
	//TODO this is for testing only
	std::vector<BWAPI::UnitType> result = {};
	if (ProductionModule::Instance()->GetCountOf(BWAPI::UnitTypes::Terran_Machine_Shop) > 0)
		result.emplace_back(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode);
	if (ProductionModule::Instance()->GetCountOf(BWAPI::UnitTypes::Terran_Factory) > 0 && BWAPI::Broodwar->self()->gas() < 400)
		result.emplace_back(BWAPI::UnitTypes::Terran_Vulture);
	if (ProductionModule::Instance()->GetCountOf(BWAPI::UnitTypes::Terran_Barracks) > 0)
		result.emplace_back(BWAPI::UnitTypes::Terran_Marine);

	return result;
}

BWAPI::UnitType StrategyModule::GetMacroProductionType()
{
	//TODO this is for testing only
	if ((ProductionModule::Instance()->GetCountOf(BWAPI::UnitTypes::Terran_Barracks) > 0)
		&& ((ProductionModule::Instance()->GetCountOf(BWAPI::UnitTypes::Terran_Barracks) * 3) >= ProductionModule::Instance()->GetCountOf(BWAPI::UnitTypes::Terran_Factory)))
	{
		if (ProductionModule::Instance()->GetCountOf(BWAPI::UnitTypes::Terran_Refinery) > 0)
		{
			if (ProductionModule::Instance()->GetCountOf(BWAPI::UnitTypes::Terran_Machine_Shop) < ProductionModule::Instance()->GetCountOf(BWAPI::UnitTypes::Terran_Factory))
				return BWAPI::UnitTypes::Terran_Machine_Shop;

			return BWAPI::UnitTypes::Terran_Factory;
		}
	}
	return BWAPI::UnitTypes::Terran_Barracks;
}

