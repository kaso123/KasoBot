#pragma once
#include "Unit.h"

namespace BWEM {
	class Mineral;
}

namespace KasoBot {
	namespace Workers {
		enum Role {
			IDLE,
			MINERALS,
			GAS,
			ASSIGNED,
			BUILDING
		};
	}
	
	class Worker : public KasoBot::Unit
	{
	private:
		Workers::Role _workerRole;
		BWEM::Mineral* _mineral;
		BWAPI::Unit _refinery;

	public:
		Worker(BWAPI::Unit unit);
		~Worker();

		//send worker to mine minerals and assign mineral patch
		void AssignRoleMinerals(BWEM::Mineral* mineral);

		//send worker to mine gas and assign refinery
		void AssignRoleGas(BWAPI::Unit refinery);


		//getters and setters
		Workers::Role GetWorkerRole() { return _workerRole; };
		void SetWorkerRole(Workers::Role newRole) { _workerRole = newRole; };
	};
}


