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

		Opener* _activeOpener;
		std::string _activeOpenerName;

	public:
		static StrategyModule* Instance();

		void EnemyDestroyed(BWAPI::UnitType type);

		//add new opener instance to list of openers
		void NewOpener(const std::string& name, nlohmann::json& array);

		//make this opener active, if it doesn't exists, choose random opener
		//@param name = can have value of "random" which chooses randomly from list of openers
		void SetOpener(const std::string& name);

		//getters and setters

		int EnemyLostMinerals() const { return _enemyLostMinerals; }
		int EnemyLostGas() const { return _enemyLostGas; }
		const std::string& GetOpenerName() const { return _activeOpenerName; }
	};
}


