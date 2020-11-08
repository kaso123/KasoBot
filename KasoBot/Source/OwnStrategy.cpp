#include "OwnStrategy.h"
#include "StrategyModule.h"
#include "ProductionModule.h"
#include "Config.h"
#include "Unit.h"

using namespace KasoBot;

Production::TechMacro OwnStrategy::GetMacroAfterTechPathDone() const
{
	//unlock units
	for (auto& unit : _units)
	{
		if (unit._proportion < 0.0f)
		{
			BWAPI::UnitType req = Config::Utils::NextPrerequisite(unit._type);
			if (req != unit._type)
				return Production::TechMacro(req);
		}
	}

	//finish upgrades
	for (auto& item : _tech)
	{
		if (item._upgrade == BWAPI::UpgradeTypes::None)
			continue;
		if (BWAPI::Broodwar->self()->isUpgrading(item._upgrade)) //skip in progress
			continue;
		if (BWAPI::Broodwar->self()->getUpgradeLevel(item._upgrade) == item._upgrade.maxRepeats()) //skip max level
			continue;

		BWAPI::UnitType req = Config::Utils::NextPrerequisite(item._upgrade);
		if (req != BWAPI::UnitTypes::None)
			return Production::TechMacro(req);
		
		//double up on upgrade buildings
		if (item._upgrade.maxRepeats() > 1 && ProductionModule::Instance()->GetCountOf(item._upgrade.whatUpgrades()) < 2)
		{
			auto it = ProductionModule::Instance()->Buildings().find(item._upgrade.whatUpgrades());
			if (it != ProductionModule::Instance()->Buildings().end())
			{
				//if the only building is upgrading sth else build another
				if (!it->second.empty() && it->second.front()->GetPointer()->isUpgrading()) 
				{
					return Production::TechMacro(item._upgrade.whatUpgrades());
				}
			}
			else return Production::TechMacro(item._upgrade.whatUpgrades());
		}

		return Production::TechMacro(item._upgrade);
	}

	//anything for units that we use
	for (auto& it = _units.rbegin(); it != _units.rend(); it++)
	{
		for (auto& upgrade : it->_type.upgrades())
		{
			if (upgrade == BWAPI::UpgradeTypes::None)
				continue;
			if (BWAPI::Broodwar->self()->isUpgrading(upgrade)) //skip in progress
				continue;
			if (BWAPI::Broodwar->self()->getUpgradeLevel(upgrade) == upgrade.maxRepeats()) //skip max level
				continue;

			BWAPI::UnitType req = Config::Utils::NextPrerequisite(upgrade);
			if (req != BWAPI::UnitTypes::None)
				return Production::TechMacro(req);

			//double up on upgrade buildings
			if (upgrade.maxRepeats() > 1 && ProductionModule::Instance()->GetCountOf(upgrade.whatUpgrades()) < 2)
			{
				auto it = ProductionModule::Instance()->Buildings().find(upgrade.whatUpgrades());
				if (it != ProductionModule::Instance()->Buildings().end())
				{
					//if the only building is upgrading sth else build another
					if (!it->second.empty() && it->second.front()->GetPointer()->isUpgrading())
					{
						return Production::TechMacro(upgrade.whatUpgrades());
					}
				}
				else return Production::TechMacro(upgrade.whatUpgrades());
			}
			return Production::TechMacro(upgrade);
		}
		for (auto& tech : BWAPI::TechTypes::allTechTypes()) //cycle through tech and check if anything is useful for this unit
		{
			if (BWAPI::Broodwar->self()->hasResearched(tech))
				continue;
			if (BWAPI::Broodwar->self()->isResearching(tech))
				continue;
			if (tech.whatUses().find(it->_type) == tech.whatUses().end())
				continue;

			BWAPI::UnitType prereq = Config::Utils::NextPrerequisite(tech);
			if (prereq != BWAPI::UnitTypes::None)
				return Production::TechMacro{ prereq }; //build prerequisites before, if needed

			return Production::TechMacro{ tech };
		}
	}

	//everything is done for tech
	return Production::TechMacro(BWAPI::UnitTypes::None);
}

OwnStrategy::OwnStrategy(std::string& name, std::string& opener, nlohmann::json& data)
	:_name(name), _opener(opener), _data(std::move(data))
{
}

OwnStrategy::~OwnStrategy()
{
}

void OwnStrategy::AddUnit(BWAPI::UnitType type, int count)
{
	_units.emplace_back(UnitItem(type,count));
}

void OwnStrategy::AddTech(Production::TechMacro macro)
{
	_tech.emplace_back(macro);
}

void OwnStrategy::CalculateProduction()
{
	int barracks = 0;
	int factory = 0;
	int starport = 0;
	int machine = 0;
	int tower = 0;

	for (auto& item : _units)
	{
		if (item._type.whatBuilds().first == BWAPI::UnitTypes::Terran_Barracks)
			barracks += item._value;
		if (item._type.whatBuilds().first == BWAPI::UnitTypes::Terran_Factory)
			factory += item._value;
		if (item._type.whatBuilds().first == BWAPI::UnitTypes::Terran_Starport)
			starport += item._value;
		if (item._type == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode)
			machine += item._value;
		if (item._type.isFlyer() && item._type != BWAPI::UnitTypes::Terran_Wraith)
			tower += item._value;
	}

	if (barracks > 0)
		_production.emplace_back(UnitItem(BWAPI::UnitTypes::Terran_Barracks, barracks));
	if (factory > 0)
		_production.emplace_back(UnitItem(BWAPI::UnitTypes::Terran_Factory, factory));
	if (starport > 0)
		_production.emplace_back(UnitItem(BWAPI::UnitTypes::Terran_Starport, starport));
	if (machine > 0)
		_production.emplace_back(UnitItem(BWAPI::UnitTypes::Terran_Machine_Shop, machine));
	if (tower > 0)
		_production.emplace_back(UnitItem(BWAPI::UnitTypes::Terran_Control_Tower, tower));
}

std::vector<BWAPI::UnitType> OwnStrategy::GetMacroArmyTypes()
{
	std::vector<BWAPI::UnitType> result;

	//temporary variable excluding units that we can't build
	int totalExpected = 0;
	int totalArmy = 0;
	for (auto& item : _units)
	{
		if (Config::Utils::CanBuild(item._type))
		{
			totalExpected += item._value;
			totalArmy += ProductionModule::Instance()->GetCountOf(item._type);
			item._proportion = 0.0f;
		}
		else item._proportion = -1.0f;
	}

	//calculate ratio for each type
	for (auto& item : _units)
	{
		if (item._proportion < 0.0f) //skip locked units
			continue;

		int typeCount = ProductionModule::Instance()->GetCountOf(item._type);
		//TODO also add inProgress units
		if (totalArmy <= 0 || typeCount <= 0)
		{
			continue; //if no unit exists, keep proportion to 0
		}

		float totalRatio = totalExpected / (float)totalArmy;
		item._proportion = (totalRatio * typeCount) / (float)item._value;
	}


	//sort result
	std::sort(std::begin(_units), std::end(_units),
		[](const KasoBot::UnitItem& a, const KasoBot::UnitItem&  b)
		{ //sort function for UnitItem
			if (a._proportion < b._proportion) //lower proportion is first
				return true;
			else if (a._proportion == b._proportion)
			{
				if (a._value > b._value) //higher count is first
					return true;
				return false;
			}
			return false;
		});

	//select types to build
	for (auto& item : _units)
	{
		if (item._proportion < 0.0f)
			continue;
		if (item._proportion > Config::Production::MaxUnitProportion())
			break;

		result.emplace_back(item._type);
	}
	return result;
}

BWAPI::UnitType OwnStrategy::GetMacroProductionType()
{
	//temporary variable excluding buildings that we can't build
	int totalExpected = 0;
	int totalBuildings = 0;
	for (auto& item : _production)
	{
		if (Config::Utils::NextPrerequisite(item._type) == item._type)
		{
			totalExpected += item._value;
			totalBuildings += ProductionModule::Instance()->GetCountOf(item._type);
			item._proportion = 0.0f;
		}
		else item._proportion = -1.0f;
	}

	//calculate ratio for each type
	for (auto& item : _production)
	{
		if (item._proportion < 0.0f) //skip locked units
			continue;

		int typeCount = ProductionModule::Instance()->GetCountOf(item._type);
		if (totalBuildings <= 0 || typeCount <= 0)
		{
			continue; //if no unit exists, keep proportion to 0
		}

		float totalRatio = totalExpected / (float)totalBuildings;
		item._proportion = (totalRatio * typeCount) / (float)item._value;
	}


	//sort result
	std::sort(std::begin(_production), std::end(_production),
		[](const KasoBot::UnitItem& a, const KasoBot::UnitItem&  b)
		{ //sort function for UnitItem
			if (a._proportion < b._proportion) //lower proportion is first
				return true;
			else if (a._proportion == b._proportion)
			{
				if (a._value > b._value) //higher count is first
					return true;
				return false;
			}
			return false;
		});

	//select types to build
	if (!_production.empty())
		return Config::Utils::NextPrerequisite(_production.front()._type);

	return BWAPI::UnitTypes::None;
}

Production::TechMacro OwnStrategy::GetMacroTechType() const
{
	for (auto it = _tech.begin(); it != _tech.end(); it++)
	{
		if ((*it)._unit != BWAPI::UnitTypes::None)
		{
			//count how many instances of this building we want to have
			int count = 1;
			for (auto it_sub = _tech.begin(); it_sub != it; it_sub++)
			{
				if ((*it)._unit == (*it_sub)._unit)
					count++;
			}

			//check if this item is satisfied
			if (ProductionModule::Instance()->GetCountOf((*it)._unit) >= count)
				continue;
			
			return Production::TechMacro{ Config::Utils::NextPrerequisite((*it)._unit) };
		}
		else if ((*it)._tech != BWAPI::TechTypes::None)
		{
			if (BWAPI::Broodwar->self()->hasResearched((*it)._tech))
				continue;
			if (BWAPI::Broodwar->self()->isResearching((*it)._tech))
				continue;

			BWAPI::UnitType prereq = Config::Utils::NextPrerequisite((*it)._tech);
			if(prereq != BWAPI::UnitTypes::None)
				return Production::TechMacro{ prereq }; //build prerequisites before, if needed

			return Production::TechMacro{ (*it)._tech };
		}
		else if ((*it)._upgrade != BWAPI::UpgradeTypes::None)
		{
			int level = 1; //count which level is this
			if ((*it)._upgrade.maxRepeats() > 1)
			{
				for (auto it_sub = _tech.begin(); it_sub != it; it_sub++)
				{
					if ((*it)._upgrade == (*it_sub)._upgrade)
						level++;
				}
			}

			if (level > (*it)._upgrade.maxRepeats())
				continue;
			if (BWAPI::Broodwar->self()->getUpgradeLevel((*it)._upgrade) >= level)
				continue;
			if ((BWAPI::Broodwar->self()->getUpgradeLevel((*it)._upgrade) >= level - 1) //is already in progress
				&& (BWAPI::Broodwar->self()->isUpgrading((*it)._upgrade)))
				continue;

			BWAPI::UnitType prereq = Config::Utils::NextPrerequisite((*it)._upgrade);
			if (prereq != BWAPI::UnitTypes::None)
				return Production::TechMacro{ prereq }; //build prerequisites before, if needed

			return Production::TechMacro{ (*it)._upgrade };
		}
	}
	
	return GetMacroAfterTechPathDone();
}

int OwnStrategy::MaxArmySupply() const
{
	return _data.contains("maxArmySupply") ? _data["maxArmySupply"] : Config::Strategy::MaxArmySupply();
}

int KasoBot::OwnStrategy::MaxAttackTasks() const
{
	return _data.contains("maxAttackTasks") ? _data["maxAttackTasks"] : Config::Strategy::MaxAttackTasks();
}
