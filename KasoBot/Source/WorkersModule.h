#pragma once
#include <BWAPI.h>

namespace KasoBot {
	
	class Expansion;

	class WorkersModule
	{
	private:
		WorkersModule();
		~WorkersModule();
		static WorkersModule* _instance;

		std::vector<std::shared_ptr<Expansion>> _expansionList;

		//@return closest expansion that is not saturated or nullptr if worker should be added to military
		std::shared_ptr<Expansion> FindExpansionForWorker(BWAPI::Unit unit);

		//get workers from army or from saturated bases and transfer them here
		void AssignIdleWorkers(std::shared_ptr<Expansion> exp);

	public:
		static WorkersModule* Instance();
		void OnStart();
		void OnFrame();

		//find expansion for created worker and assign role
		void NewWorker(BWAPI::Unit unit);

		//remove worker from expansion
		void RemoveWorker(BWAPI::Unit unit);

		void ExpansionCreated(BWAPI::Unit unit);

		void ExpansionDestroyed(BWAPI::Unit unit);
	};
}


