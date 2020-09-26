#include "Config.h"
#include "StrategyModule.h"
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
		_hiddenPositionResetFrames = j["units"].contains("hiddenPositionReset") ? j["units"]["hiddenPositionReset"] : _hiddenPositionResetFrames;
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
	if (j.contains("strategy"))
	{
		_maxArmySupply = j["strategy"].contains("maxArmySupply") ? j["strategy"]["maxArmySupply"] : _maxArmySupply;

		if (j["strategy"].contains("opener"))
		{
			//be sure to call this after openers have been loaded
			StrategyModule::Instance()->SetOpener(j["strategy"]["opener"]);
		}
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
int Config::Units::HiddenPositionResetFrames() { return ConfigModule::Instance()->HiddenPositionResetFrames(); }

float Config::Production::FreeSupplyMultiplier() { return ConfigModule::Instance()->FreeSupplyMultiplier(); }

bool Config::Debug::Map() { return ConfigModule::Instance()->DebugMap(); }
bool Config::Debug::Workers() { return ConfigModule::Instance()->DebugWorkers(); }
bool Config::Debug::Army() { return ConfigModule::Instance()->DebugArmy(); }
bool Config::Debug::BuildOrder() { return ConfigModule::Instance()->DebugBuildOrder(); }
bool Config::Debug::Strategy() { return ConfigModule::Instance()->DebugStrategy(); }
bool Config::Debug::Orders() { return ConfigModule::Instance()->DebugOrders(); }
bool Config::Debug::Bases() { return ConfigModule::Instance()->DebugBases(); }
bool Config::Debug::Resources() { return ConfigModule::Instance()->DebugResources(); }
bool Config::Debug::Enemy() { return ConfigModule::Instance()->DebugEnemy(); }

int Config::Strategy::MaxArmySupply() { return ConfigModule::Instance()->MaxArmySupply(); }
int Config::Strategy::FirstScoutSupply() { return ConfigModule::Instance()->FirstScoutSupply(); }

BWAPI::UnitType Config::Utils::TypeFromString(std::string input)
{
	//aliases for some unit types
	if (input == "depot")
		input = "supply_depot";
	else if (input == "cc")
		input = "command_center";

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
