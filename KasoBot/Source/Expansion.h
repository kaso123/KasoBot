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
		void RemoveWorker(BWAPI::Unit unit);
	};
}


