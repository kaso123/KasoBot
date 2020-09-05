#pragma once
#include "Unit.h"

namespace BWEM {
	class Mineral;
}

namespace KasoBot {

	class ProductionItem;

	namespace Workers {
		enum Role {
			IDLE,
			MINERALS,
			GAS,
			ASSIGNED,
			BUILD
		};
	}
	
	class Worker : public KasoBot::Unit
	{
	private:
		Workers::Role _workerRole;
		BWEM::Mineral* _mineral;
		BWAPI::Unit _refinery;

		ProductionItem* _item;

	public:
		Worker(BWAPI::Unit unit);
		~Worker();

		//send worker to mine minerals and assign mineral patch
		void AssignRoleMinerals(BWEM::Mineral* mineral);

		//send worker to mine gas and assign refinery
		void AssignRoleGas(BWAPI::Unit refinery);

		//add production item to worker and make him build building
		bool AssignRoleBuild(ProductionItem* item);
		
		//check if worker has this mineral assigned
		bool IsMiningMineral(BWAPI::Unit mineral);

		//set build item to nullptr
		void BuildFinished();

		void Work() override;


		//getters and setters

		Workers::Role GetWorkerRole() const { return _workerRole; };
		void SetWorkerRole(Workers::Role newRole) { _workerRole = newRole; };
		const BWEM::Mineral* GetMineral() const { return _mineral; }
		const BWAPI::Unit GetRefinery() const { return _refinery; }
		ProductionItem* GetProductionItem() const { return _item; }
	};
}


