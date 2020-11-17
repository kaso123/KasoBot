#pragma once
#include <BWAPI.h>
#include "libs/nlohmann/json.hpp"


namespace KasoBot {
	
	class EnemyStrategy;
	class OwnStrategy;

	namespace Production {
		//things we cycle through in some order when figuring out what to do next
		//order of these items can be specified in config
		enum Type {
			SATURATION,
			ARMY,
			PRODUCTION,
			TECH
		};

		struct TechMacro {
			BWAPI::UnitType _unit;
			BWAPI::UpgradeType _upgrade;
			BWAPI::TechType _tech;
			TechMacro(BWAPI::UnitType type)
				:_unit(type),_upgrade(BWAPI::UpgradeTypes::None), _tech(BWAPI::TechTypes::None) {};
			TechMacro(BWAPI::UpgradeType type)
				:_unit(BWAPI::UnitTypes::None), _upgrade(type), _tech(BWAPI::TechTypes::None) {};
			TechMacro(BWAPI::TechType type)
				:_unit(BWAPI::UnitTypes::None), _upgrade(BWAPI::UpgradeTypes::None), _tech(type) {};
		};
	}
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
		std::map<std::string, std::unique_ptr<OwnStrategy>> _strategies;

		std::vector<std::unique_ptr<EnemyStrategy>> _stratsT;
		std::vector<std::unique_ptr<EnemyStrategy>> _stratsP;
		std::vector<std::unique_ptr<EnemyStrategy>> _stratsZ;
		
		std::vector<Production::Type> _productionCycle;

		Opener* _activeOpener;
		std::string _activeOpenerName;

		OwnStrategy* _activeStrat;
		std::string _activeStratName;

		EnemyStrategy* _activeEnemyStrat;

		//try to build workers
		bool MacroSaturation();

		//try to make units
		bool MacroArmy();

		//try to do tech/upgrades + build new tech buildings
		bool MacroTech();

		//try to build more production buildings
		bool MacroProduction();

		//cycle through all possible enemy strategies and choose which has best score
		void CheckEnemyStrat();

		//choose new strategy that counters enemy strategy
		void ChooseNewStrat();

	public:
		static StrategyModule* Instance();

		void OnFrame();

		void EnemyDestroyed(BWAPI::UnitType type);

		//add new opener instance to list of openers
		void NewOpener(const std::string& name, nlohmann::json& array);

		//make this opener active, if it doesn't exists, choose random opener
		//@param name = can have value of "random" which chooses randomly from list of openers
		void SetOpener(const std::string& name);

		//make this strategy active
		//@param name = can have value of "random" which chooses randomly from list of strategies
		void SetStrategy(const std::string& name);

		//set order of saturation/army/production/tech
		void SetCycle(nlohmann::json& itemsArray);

		//parse new enemy strategy from json
		void NewEnemyStrategy(BWAPI::Race race, nlohmann::json& strat, int id);

		//parse new strategy from json
		void NewOwnStrategy(nlohmann::json& strat);
		
		//return list of possible enemy strategies according to his race
		const std::vector<std::unique_ptr<EnemyStrategy>>& GetEnemyStrategies();

		//return current active cycle, either from strategy or default
		const std::vector<Production::Type>& GetCycle() const;

		//change used strategy, also check if everything in opener is done
		void SwitchStrategy(OwnStrategy* newStrat, const std::string& name);

		//change used strategy, also check if everything in opener is done
		void SwitchOpener(Opener* newOpener, const std::string& name);

		//insert this type into opener again
		void AddToOpener(BWAPI::UnitType type);

		//getters and setters

		int EnemyLostMinerals() const { return _enemyLostMinerals; }
		int EnemyLostGas() const { return _enemyLostGas; }
		const std::string& GetOpenerName() const { return _activeOpenerName; }
		const std::string& GetStratName() const { return _activeStratName; }
		const OwnStrategy* GetActiveStrat() const { return _activeStrat; }
		const EnemyStrategy* GetEnemyStrat() const { return _activeEnemyStrat; }
		bool IsOpenerActive() { return _activeOpener; }
	};
}


