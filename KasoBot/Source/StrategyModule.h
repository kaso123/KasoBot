#pragma once
#include <BWAPI.h>
#include "libs/nlohmann/json.hpp"


namespace KasoBot {

	class Opener;

	class StrategyModule
	{
	private:
		StrategyModule();
		~StrategyModule();
		static StrategyModule* _instance;

		int _enemyLostMinerals;
		int _enemyLostGas;

		std::map<std::string, std::unique_ptr<Opener>> _openers;

	public:
		static StrategyModule* Instance();

		void EnemyDestroyed(BWAPI::UnitType type);

		void NewOpener(const std::string& name, nlohmann::json& array);

		//getters and setters

		int EnemyLostMinerals() const { return _enemyLostMinerals; }
		int EnemyLostGas() const { return _enemyLostGas; }
	};
}


