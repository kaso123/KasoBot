#include "Config.h"
#include "StrategyModule.h"
#include "ProductionModule.h"

#include "libs/nlohmann/json.hpp"
#include <fstream>

using namespace KasoBot;

ConfigModule* ConfigModule::_instance = 0;

ConfigModule::ConfigModule()
{
}

ConfigModule::~ConfigModule()
{
	delete(_instance);
}

ConfigModule* ConfigModule::Instance()
{
	if (!_instance)
		_instance = new ConfigModule;
	return _instance;
}

void ConfigModule::Init()
{
	// read a JSON file
	std::ifstream stream("bwapi-data/AI/KasoBot.json");
	_ASSERT(stream);
	nlohmann::json j;
	stream >> j;

	if (j.contains("workers"))
	{
		_maxWorkersPerMineral = j["workers"].contains("maxPerMineral") ? j["workers"]["maxPerMineral"] : _maxWorkersPerMineral;
		_maxWorkersPerGas = j["workers"].contains("maxPerGas") ? j["workers"]["maxPerGas"] : _maxWorkersPerGas;
		_maxWorkersTotal = j["workers"].contains("max") ? j["workers"]["max"] : _maxWorkersTotal;
		_saturatedMineral = j["workers"].contains("saturatedMineral") ? j["workers"]["saturatedMineral"] : _saturatedMineral;
		_saturatedGas = j["workers"].contains("saturatedGas") ? j["workers"]["saturatedGas"] : _saturatedGas;
		_startGasAfter = j["workers"].contains("startGasAfter") ? j["workers"]["startGasAfter"] : _startGasAfter;	
		_buildStartDistance = j["workers"].contains("buildStartDistance") ? j["workers"]["buildStartDistance"] : _buildStartDistance;
		_workerResourceValue = j["workers"].contains("workerResourceValue") ? j["workers"]["workerResourceValue"] : _workerResourceValue;
	}
	if (j.contains("units"))
	{
		_unitOrderDelay = j["units"].contains("orderDelay") ? j["units"]["orderDelay"] : _unitOrderDelay;
		_clearTileLock = j["units"].contains("clearTileLock") ? j["units"]["clearTileLock"] : _clearTileLock;
		_enemyPositionResetFrames = j["units"].contains("enemyPositionReset") ? j["units"]["enemyPositionReset"] : _enemyPositionResetFrames;
		_enemyArmyRange = j["units"].contains("enemyArmyRange") ? j["units"]["enemyArmyRange"] : _enemyArmyRange;
		_armyRange = j["units"].contains("armyRange") ? j["units"]["armyRange"] : _armyRange;
		_hiddenPositionResetFrames = j["units"].contains("hiddenPositionReset") ? j["units"]["hiddenPositionReset"] : _hiddenPositionResetFrames;
		_hiddenBaseResetFrames = j["units"].contains("hiddenBaseReset") ? j["units"]["hiddenBaseReset"] : _hiddenBaseResetFrames;
		_scoutBaseRadius = j["units"].contains("scoutBaseRadius") ? j["units"]["scoutBaseRadius"] : _scoutBaseRadius;
		_enemyThreatRadius = j["units"].contains("enemyThreatRadius") ? j["units"]["enemyThreatRadius"] : _enemyThreatRadius;
	}
	if (j.contains("production"))
	{
		_freeSupplyMultiplier = j["production"].contains("freeSupplyMultiplier") ? j["production"]["freeSupplyMultiplier"] : _freeSupplyMultiplier;
	}
	if (j.contains("debug"))
	{
		_debugMap = j["debug"].contains("map") ? j["debug"]["map"] : _debugMap;
		_debugWorkers = j["debug"].contains("workers") ? j["debug"]["workers"] : _debugWorkers;
		_debugArmy = j["debug"].contains("army") ? j["debug"]["army"] : _debugArmy;
		_debugTasks = j["debug"].contains("tasks") ? j["debug"]["tasks"] : _debugTasks;
		_debugBuildOrder = j["debug"].contains("build") ? j["debug"]["build"] : _debugBuildOrder;
		_debugStrategy = j["debug"].contains("strategy") ? j["debug"]["strategy"] : _debugStrategy;
		_debugOrders = j["debug"].contains("orders") ? j["debug"]["orders"] : _debugOrders;
		_debugBases = j["debug"].contains("bases") ? j["debug"]["bases"] : _debugBases;
		_debugResources = j["debug"].contains("resources") ? j["debug"]["resources"] : _debugResources;
		_debugEnemy = j["debug"].contains("enemyInfo") ? j["debug"]["enemyInfo"] : _debugEnemy;
	}
	if (j.contains("openers"))
	{
		for (auto it = j["openers"].begin(); it != j["openers"].end(); ++it)
		{
			StrategyModule::Instance()->NewOpener(it.key(), it.value());
		}
	}
	if (j.contains("strategies") && j["strategies"].is_array())
	{
		//parse strats
		for (auto& strat : j["strategies"])
		{
			StrategyModule::Instance()->NewOwnStrategy(strat);
		}
	}
	if (j.contains("strategy"))
	{
		_maxArmySupply = j["strategy"].contains("maxArmySupply") ? j["strategy"]["maxArmySupply"] : _maxArmySupply;
		_maxAttackTasks = j["strategy"].contains("maxAttackTasks") ? j["strategy"]["maxAttackTasks"] : _maxAttackTasks;
		_maxScoutTasks = j["strategy"].contains("maxScoutTasks") ? j["strategy"]["maxScoutTasks"] : _maxScoutTasks;
		_skipOpenerAt = j["strategy"].contains("skipOpenerAtFrame") ? j["strategy"]["skipOpenerAtFrame"] : _skipOpenerAt;

		StrategyModule::Instance()->SetStrategy( j["strategy"].contains("default") ? j["strategy"]["default"] : "random");

		if (j["strategy"].contains("cycle"))
		{
			StrategyModule::Instance()->SetCycle(j["strategy"]["cycle"]);
		}
		if (j["strategy"].contains("scout"))
		{
			if (j["strategy"]["scout"].contains("first"))
			{
				if (BWAPI::Broodwar->getStartLocations().size() >= 4)
				{
					_firstScoutSupply = j["strategy"]["scout"]["first"].contains("4starts") ? j["strategy"]["scout"]["first"]["4starts"] : _firstScoutSupply;
				}
				else if (BWAPI::Broodwar->getStartLocations().size() == 3)
				{
					_firstScoutSupply = j["strategy"]["scout"]["first"].contains("3starts") ? j["strategy"]["scout"]["first"]["3starts"] : _firstScoutSupply;
				}
				else
				{
					_firstScoutSupply = j["strategy"]["scout"]["first"].contains("2starts") ? j["strategy"]["scout"]["first"]["2starts"] : _firstScoutSupply;
				}
			}
		}
	}
	//load known enemy strategies
	if (j.contains("enemyStrategies"))
	{
		int i = 1;
		for (auto it = j["enemyStrategies"].begin(); it != j["enemyStrategies"].end(); it++)
		{
			BWAPI::Race race;
			if (it.key() == "terran")
				race = BWAPI::Races::Terran;
			else if (it.key() == "protoss")
				race = BWAPI::Races::Protoss;
			else if (it.key() == "zerg")
				race = BWAPI::Races::Zerg;
			else continue;

			if (!it.value().is_array())
				continue;

			for (auto& strat : it.value())
			{
				if (!strat.contains("name") || !strat.contains("types")
					|| !strat["types"].is_array())
					continue;

				StrategyModule::Instance()->NewEnemyStrategy(race, strat, i++);
			}
		}
	}
}


int Config::Workers::MaxGlobal() { return ConfigModule::Instance()->MaxWorkersTotal(); }
int Config::Workers::MaxPerMineral() { return ConfigModule::Instance()->MaxWorkersPerMineral(); }
int Config::Workers::MaxPerGas() { return ConfigModule::Instance()->MaxWorkersPerGas(); }
int Config::Workers::SaturationPerMineral() { return ConfigModule::Instance()->SaturatedMineral(); }
int Config::Workers::SaturationPerGas() { return ConfigModule::Instance()->SaturatedGas(); }
int Config::Workers::StartGasAfter() { return ConfigModule::Instance()->StartGasAfter(); }
int Config::Workers::BuildStartDistance() { return ConfigModule::Instance()->BuildStartDistance(); }
int Config::Workers::WorkerResourceValue() { return ConfigModule::Instance()->WorkerResourceValue(); }

int Config::Units::OrderDelay() { return ConfigModule::Instance()->UnitOrderDelay(); }
int Config::Units::ClearTileLock() { return ConfigModule::Instance()->ClearTileLock(); }
int Config::Units::EnemyPositionResetFrames() { return ConfigModule::Instance()->EnemyPositionResetFrames(); }
int Config::Units::EnemyArmyRange() { return ConfigModule::Instance()->EnemyArmyRange(); }
int Config::Units::ArmyRange() { return ConfigModule::Instance()->ArmyRange(); }
int Config::Units::HiddenPositionResetFrames() { return ConfigModule::Instance()->HiddenPositionResetFrames(); }
int Config::Units::HiddenBaseResetFrames() { return ConfigModule::Instance()->HiddenBaseResetFrames(); }
int Config::Units::ScoutBaseRadius() { return ConfigModule::Instance()->ScoutBaseRadius(); }
int Config::Units::EnemyThreatRadius() { return ConfigModule::Instance()->EnemyThreatRadius(); }

float Config::Production::FreeSupplyMultiplier() { return ConfigModule::Instance()->FreeSupplyMultiplier(); }

bool Config::Debug::Map() { return ConfigModule::Instance()->DebugMap(); }
bool Config::Debug::Workers() { return ConfigModule::Instance()->DebugWorkers(); }
bool Config::Debug::Army() { return ConfigModule::Instance()->DebugArmy(); }
bool Config::Debug::Tasks() { return ConfigModule::Instance()->DebugTasks(); }
bool Config::Debug::BuildOrder() { return ConfigModule::Instance()->DebugBuildOrder(); }
bool Config::Debug::Strategy() { return ConfigModule::Instance()->DebugStrategy(); }
bool Config::Debug::Orders() { return ConfigModule::Instance()->DebugOrders(); }
bool Config::Debug::Bases() { return ConfigModule::Instance()->DebugBases(); }
bool Config::Debug::Resources() { return ConfigModule::Instance()->DebugResources(); }
bool Config::Debug::Enemy() { return ConfigModule::Instance()->DebugEnemy(); }

int Config::Strategy::MaxArmySupply() { return ConfigModule::Instance()->MaxArmySupply(); }
int Config::Strategy::MaxAttackTasks() { return ConfigModule::Instance()->MaxAttackTasks(); }
int Config::Strategy::MaxScoutTasks() { return ConfigModule::Instance()->MaxScoutTasks(); }
int Config::Strategy::FirstScoutSupply() { return ConfigModule::Instance()->FirstScoutSupply(); }
int Config::Strategy::SkipOpenerAt() { return ConfigModule::Instance()->SkipOpenerAt(); }

namespace {
	std::map<std::string, std::string> aliases{
		{"depot", "supply_depot"},
		{"cc", "command_center"},
		{"tank", "siege_tank_tank_mode"},
		{"core", "cybernetics_core"},
		{"archives", "templar_archives"},
		{"citadel", "citadel_of_adun"},
		{"pool", "spawning_pool"},
		{"siege", "tank_siege_mode"},
		{"mines", "spider_mines"},
		{"yamato", "yamato_gun"},
		{"stim", "stim_packs"},
		{"u238", "u_238_shells"},
		{"infantry_weapons", "terran_infantry_weapons"},
		{"infantry_armor", "terran_infantry_armor"},
		{"vehicle_weapons", "terran_vehicle_weapons"},
		{"vehicle_plating", "terran_vehicle_plating"},
		{"ship_weapons", "terran_ship_weapons"},
		{"ship_plating", "terran_ship_plating"}
	};

	void Alias(std::string& name)
	{
		if (aliases.find(name) != aliases.end())
		{
			name = aliases[name];
		}
	}
}

BWAPI::UnitType Config::Utils::TypeFromString(std::string input)
{
	Alias(input);

	//cycle all types
	for (const auto& type : BWAPI::UnitTypes::allUnitTypes())
	{
		//compare without race part of string
		const std::string& race = type.getRace().getName();
		std::string name = type.getName();
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);
		
		if ((name.length() > race.length()) && (name.compare(race.length() + 1, name.length(), input) == 0))
		{
			return type;
		}
	}

	_ASSERT(false);
	return BWAPI::UnitTypes::None;
}

KasoBot::Production::TechMacro Config::Utils::TechTypeFromString(std::string input)
{
	Alias(input);

	//cycle unit types
	for (const auto& type : BWAPI::UnitTypes::allUnitTypes())
	{
		//compare without race part of string
		const std::string& race = type.getRace().getName();
		std::string name = type.getName();
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);

		if ((name.length() > race.length()) && (name.compare(race.length() + 1, name.length(), input) == 0))
		{
			return KasoBot::Production::TechMacro(type);
		}
	}
	//cycle tech types
	for (const auto& type : BWAPI::TechTypes::allTechTypes())
	{
		std::string name = type.getName();
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);

		if (name.compare(input) == 0)
		{
			return KasoBot::Production::TechMacro(type);
		}
	}
	//cycle tech types
	for (const auto& type : BWAPI::UpgradeTypes::allUpgradeTypes())
	{
		std::string name = type.getName();
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);

		if (name.compare(input) == 0)
		{
			return KasoBot::Production::TechMacro(type);
		}
	}

	_ASSERT(false);
	return KasoBot::Production::TechMacro(BWAPI::UnitTypes::None);
}

BWAPI::UnitType Config::Utils::NextPrerequisite(BWAPI::UnitType type)
{
	for (auto req : type.requiredUnits())
	{
		if (req.first.isWorker()) //SCV is part of requirements for buildings, skip that
			continue;

		if (ProductionModule::Instance()->GetCountOf(req.first) < req.second)
			return Config::Utils::NextPrerequisite(req.first); //recursively check requirements
	}
	return type;
}

BWAPI::UnitType Config::Utils::NextPrerequisite(BWAPI::TechType type)
{
	if (ProductionModule::Instance()->GetCountOf(type.whatResearches()) < 1)
			return Config::Utils::NextPrerequisite(type.whatResearches()); //recursively check requirements

	return BWAPI::UnitTypes::None;
}

BWAPI::UnitType Config::Utils::NextPrerequisite(BWAPI::UpgradeType type)
{
	_ASSERT(!BWAPI::Broodwar->self()->isUpgrading(type));

	int currLevel = BWAPI::Broodwar->self()->getUpgradeLevel(type);
	_ASSERT(currLevel < BWAPI::Broodwar->self()->getMaxUpgradeLevel(type));

	BWAPI::UnitType reqType = type.whatsRequired(currLevel + 1);
	if(reqType != BWAPI::UnitTypes::None && ProductionModule::Instance()->GetCountOf(reqType) < 1)
		return Config::Utils::NextPrerequisite(type.whatsRequired(currLevel + 1)); //recursively check requirements

	if (ProductionModule::Instance()->GetCountOf(type.whatUpgrades()) < 1)
		return Config::Utils::NextPrerequisite(type.whatUpgrades()); //recursively check building which upgrades this
	return BWAPI::UnitTypes::None;
}

bool Config::Utils::CanBuild(BWAPI::UnitType type)
{
	for (auto& req : type.requiredUnits())
	{
		if (!BWAPI::Broodwar->self()->hasUnitTypeRequirement(req.first, req.second))
		{
			return false;
		}
	}
	return true;
}
