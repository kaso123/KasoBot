#pragma once
#include <BWAPI.h>
#include "libs/nlohmann/json.hpp"


namespace KasoBot {
	
	class EnemyStrategy;

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
			BWAPI::UnitType unit = BWAPI::UnitTypes::None;
			BWAPI::UpgradeType upgrade = BWAPI::UpgradeTypes::None;
			BWAPI::TechType tech = BWAPI::TechTypes::None;
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

		std::vector<std::unique_ptr<EnemyStrategy>> _stratsT;
		std::vector<std::unique_ptr<EnemyStrategy>> _stratsP;
		std::vector<std::unique_ptr<EnemyStrategy>> _stratsZ;
		
		std::vector<Production::Type> _productionCycle;

		Opener* _activeOpener;
		std::string _activeOpenerName;
		EnemyStrategy* _activeEnemyStrat;

		//try to build workers
		bool MacroSaturation();

		//try to make units
		bool MacroArmy();

		//try to do tech/upgrades + build new tech buildings
		bool MacroTech();

		//try to build more production buildings
		bool MacroProduction();

	public:
		static StrategyModule* Instance();

		void OnFrame();

		void EnemyDestroyed(BWAPI::UnitType type);

		//add new opener instance to list of openers
		void NewOpener(const std::string& name, nlohmann::json& array);

		//make this opener active, if it doesn't exists, choose random opener
		//@param name = can have value of "random" which chooses randomly from list of openers
		void SetOpener(const std::string& name);

		//set order of saturation/army/production/tech
		void SetCycle(nlohmann::json& itemsArray);

		//@return vector with next army units that should be built in order of priority
		std::vector<BWAPI::UnitType> GetMacroArmyTypes();

		//@return next production building that should be built
		BWAPI::UnitType GetMacroProductionType();

		//@return next upgrade, tech or building that should be built
		Production::TechMacro GetMacroTechType();

		//load new enemy strategy from
		void NewStrategy(BWAPI::Race race, nlohmann::json& strat, int id);
		
		//return list of possible enemy strategies according to his race
		const std::vector<std::unique_ptr<EnemyStrategy>>& GetEnemyStrategies();

		//getters and setters

		int EnemyLostMinerals() const { return _enemyLostMinerals; }
		int EnemyLostGas() const { return _enemyLostGas; }
		const std::string& GetOpenerName() const { return _activeOpenerName; }
		bool IsOpenerActive() { return _activeOpener; }
	};
}


