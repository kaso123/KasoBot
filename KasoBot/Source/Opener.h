#pragma once
#include <BWAPI.h>
#include "libs/nlohmann/json.hpp"

namespace KasoBot {

	class Opener
	{
	private:
		std::vector<BWAPI::UnitType> _queue;
	public:
		Opener(nlohmann::json& j);
		~Opener();
	};

}
