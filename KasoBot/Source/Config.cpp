#include "Config.h"
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
	}
	if (j.contains("units"))
	{
		_unitOrderDelay = j["units"].contains("orderDelay") ? j["units"]["orderDelay"] : _unitOrderDelay;
	}
	if (j.contains("debug"))
	{
		_debugMap = j["debug"].contains("map") ? j["debug"]["map"] : _debugMap;
		_debugWorkers = j["debug"].contains("workers") ? j["debug"]["workers"] : _debugWorkers;
		_debugArmy = j["debug"].contains("army") ? j["debug"]["army"] : _debugArmy;
		_debugBuildOrder = j["debug"].contains("build") ? j["debug"]["build"] : _debugBuildOrder;
		_debugStrategy = j["debug"].contains("strategy") ? j["debug"]["strategy"] : _debugStrategy;
		_debugOrders = j["debug"].contains("orders") ? j["debug"]["orders"] : _debugOrders;
	}
}


int Config::Workers::MaxGlobal() { return ConfigModule::Instance()->MaxWorkersTotal(); }
int Config::Workers::MaxPerMineral() { return ConfigModule::Instance()->MaxWorkersPerMineral(); }
int Config::Workers::MaxPerGas() { return ConfigModule::Instance()->MaxWorkersPerGas(); }
int Config::Workers::SaturationPerMineral() { return ConfigModule::Instance()->SaturatedMineral(); }
int Config::Workers::SaturationPerGas() { return ConfigModule::Instance()->SaturatedGas(); }
int Config::Workers::StartGasAfter() { return ConfigModule::Instance()->StartGasAfter(); }

int Config::Units::OrderDelay() { return ConfigModule::Instance()->UnitOrderDelay(); }

bool Config::Debug::Map() { return ConfigModule::Instance()->DebugMap(); }
bool Config::Debug::Workers() { return ConfigModule::Instance()->DebugWorkers(); }
bool Config::Debug::Army() { return ConfigModule::Instance()->DebugArmy(); }
bool Config::Debug::BuildOrder() { return ConfigModule::Instance()->DebugBuildOrder(); }
bool Config::Debug::Strategy() { return ConfigModule::Instance()->DebugStrategy(); }
bool Config::Debug::Orders() { return ConfigModule::Instance()->DebugOrders(); }

