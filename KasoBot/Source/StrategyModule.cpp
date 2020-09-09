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
	//TODO check if we need to expand
	if (WorkersModule::Instance()->WorkerCountMinerals() + WorkersModule::Instance()->WorkerCountGas() >= Config::Workers::MaxGlobal())
		return false;

	return ProductionModule::Instance()->BuildUnit(BWAPI::UnitTypes::Terran_SCV);
}

bool StrategyModule::MacroArmy()
{
	//TODO implement function to choose unit type
	return ProductionModule::Instance()->BuildUnit(BWAPI::UnitTypes::Terran_Marine);
}

bool StrategyModule::MacroTech()
{
	return false;
}

bool StrategyModule::MacroProduction()
{
	return false;
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
