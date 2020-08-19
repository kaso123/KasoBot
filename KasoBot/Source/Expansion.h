#pragma once
#include <BWAPI.h>

namespace KasoBot {
	
	class Worker;

	class Expansion
	{
	private:
		BWAPI::Unit _pointer;

		std::vector<std::shared_ptr<Worker>> _workerList;
	public:
		Expansion(BWAPI::Unit unit);
		~Expansion();

		void AddWorker(BWAPI::Unit unit);
		void AddWorker(std::shared_ptr<Worker> worker);
		void RemoveWorker(BWAPI::Unit unit);

		//@return true if all minerals and gases are saturated on ideal value from config
		bool IsSaturated();

		//@return true if all minerals and gases are saturated on max value from config
		bool IsFull();
		
		//getters and setters

		BWAPI::Unit GetPointer() { return _pointer; };
	};
}

