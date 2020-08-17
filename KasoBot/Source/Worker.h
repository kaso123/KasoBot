#pragma once
#include "Unit.h"

namespace KasoBot {
	class Worker : public Unit
	{
	private:
	public:
		Worker(BWAPI::Unit unit);
		~Worker();
	};
}


