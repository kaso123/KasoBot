#pragma once
#include <BWAPI.h>


namespace KasoBot {
	class StrategyModule
	{
	private:
		StrategyModule();
		~StrategyModule();
		static StrategyModule* _instance;

		int _enemyLostMinerals;
		int _enemyLostGas;

	public:
		static StrategyModule* Instance();

		void EnemyDestroyed(BWAPI::UnitType type);

		//getters and setters

		int EnemyLostMinerals() const { return _enemyLostMinerals; }
		int EnemyLostGas() const { return _enemyLostGas; }
	};
}


