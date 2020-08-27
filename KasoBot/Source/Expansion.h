#pragma once
#include <BWAPI.h>

namespace BWEM {
	class Mineral;
}

namespace BWEB {
	class Station;
}

namespace KasoBot {
	
	class Worker;

	class Expansion
	{
	private:
		BWAPI::Unit _pointer;
		BWEB::Station* _station;

		BWAPI::Unit _refinery;

		std::vector<std::shared_ptr<Worker>> _workerList;
		int _workersMinerals;
		int _workersGas;

		//used in debug to check that worker numbers are solid
		bool VerifyWorkers();

	public:
		Expansion(BWAPI::Unit unit);
		~Expansion();

		void AddWorker(BWAPI::Unit unit);
		void AddWorker(std::shared_ptr<Worker> worker);

		//set pointer to refinery and also check if workers can be assigned to gas mining
		void AddRefinery(BWAPI::Unit unit);
		
		//@return true if worker was from this expansion
		bool RemoveWorker(BWAPI::Unit unit);

		//@return true if refinery was from this expansion
		bool RemoveRefinery(BWAPI::Unit unit);

		//@return true if all minerals and gases are saturated on ideal value from config
		bool IsSaturated();

		//@return true if all minerals and gases are saturated on max value from config
		bool IsFull();

		//check if mineral is from this expansion and reassign workers if they were mining it
		//@return true if mineral was from this expansion
		//#param outToRemove - returning list of workers to reassign after the mineral was removed from BWEM
		bool CheckMineral(BWAPI::Unit mineral, std::vector<BWAPI::Unit>& outToRemove);

		//@return number of workers we want this base to currently have
		size_t IdealWorkerCount();
		
		//getters and setters

		BWAPI::Unit GetPointer() const { return _pointer; }
		BWEB::Station* GetStation() const { return _station; }
		const std::vector<std::shared_ptr<Worker>>& Workers() { return _workerList; }
	};
}


