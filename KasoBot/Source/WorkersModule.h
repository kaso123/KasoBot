#pragma once
#include <BWAPI.h>

namespace KasoBot {
	
	class Expansion;
	class ProductionItem;
	class Worker;

	class WorkersModule
	{
	private:
		WorkersModule();
		~WorkersModule();
		static WorkersModule* _instance;

		std::vector<std::unique_ptr<Expansion>> _expansionList;
		std::vector<BWAPI::Unit> _unassignedRefineries; //keeping a list of refineries without an expansion
		std::vector<std::shared_ptr<Worker>> _builders; //list of workers that are building stuff

		//@return closest expansion that is not saturated or nullptr if worker should be added to military
		Expansion* FindExpansionForWorker(BWAPI::Unit unit);

		//get workers from army or from saturated bases and transfer them here
		void AssignIdleWorkers(Expansion& exp);

		//check if there is built refinery for this expansion and assign it
		void AssignRefinery(Expansion& exp);

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

		//@param unassign - set to true if adding refinery from destroyed expansion, it skips checking the expansions list
		void RefineryCreated(BWAPI::Unit unit, bool unassign = false);

		void RefineryDestroyed(BWAPI::Unit unit);

		//find workers that were mining this mineral and reassign them, also send message to BWEM
		void MineralDestroyed(BWAPI::Unit unit);

		//assign this build item to worker
		//@return false if no worker was assigned
		bool Build(ProductionItem* item);

		//build addon for CommandCenter
		//@return true if building started
		bool BuildAddon(BWAPI::UnitType type);

		//reassign builder that was building this and set ProductionItem as DONE
		void FinishBuild(BWAPI::Unit unit);

		//getters and setters
		const std::vector<std::unique_ptr<Expansion>>& ExpansionList() const { return _expansionList; }
		const std::vector<std::shared_ptr<Worker>>& Builders() const { return _builders; }
	};
}


