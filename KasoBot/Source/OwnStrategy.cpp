#include "OwnStrategy.h"
#include "StrategyModule.h"
#include "ProductionModule.h"
#include "Config.h"

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

std::vector<BWAPI::UnitType> OwnStrategy::GetMacroArmyTypes() const
{
	//TODO select units by ratio
	std::vector<BWAPI::UnitType> result;
	result.emplace_back(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode);
	return result;
}

BWAPI::UnitType OwnStrategy::GetMacroProductionType() const
{
	//TODO select type according to units comp
	return BWAPI::UnitTypes::Terran_Barracks;
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
	return Production::TechMacro(BWAPI::UnitTypes::None);
}
