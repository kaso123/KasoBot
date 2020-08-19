#pragma once
#include <BWAPI.h>

namespace KasoBot {
	
	class Worker;

	class ArmyModule
	{
	private:
		ArmyModule();
		~ArmyModule();
		static ArmyModule* _instance;

		//TODO mockup for workers in army
		std::vector<std::shared_ptr<Worker>> _workers;

	public:
		static ArmyModule* Instance();

		//@param max = max amount of workers needed
		//@return vector of workers from army
		std::vector<std::shared_ptr<Worker>> GetFreeWorkers(int max);

		void AddWorker(std::shared_ptr<Worker> worker);

		//@return true if killed worker was from army units
		bool WorkerKilled(BWAPI::Unit unit);
	};
}


