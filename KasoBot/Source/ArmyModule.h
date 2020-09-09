#pragma once
#include <BWAPI.h>

namespace KasoBot {
	
	class Worker;
	class Unit;

	class ArmyModule
	{
	private:
		ArmyModule();
		~ArmyModule();
		static ArmyModule* _instance;

		//TODO mockup for workers in army
		std::vector<std::shared_ptr<Worker>> _workers;

		std::vector <KasoBot::Unit*> _soldiers;

	public:
		static ArmyModule* Instance();

		//@param max = max amount of workers needed
		//@return vector of workers from army
		std::vector<std::shared_ptr<Worker>> GetFreeWorkers(size_t max);

		void AddWorker(std::shared_ptr<Worker> worker);

		//@return true if killed worker was from army units
		bool WorkerKilled(BWAPI::Unit unit);

		void AddSoldier(KasoBot::Unit* unit);

		//remove killed soldier from army
		void SoldierKilled(KasoBot::Unit* unit);

		//@return total army supply, excluding mining workers
		int GetArmySupply();
	};
}


