#pragma once
#include <BWAPI.h>
#include "libs/nlohmann/json.hpp"

namespace KasoBot {

	class Opener
	{
	private:
		std::deque<BWAPI::UnitType> _queue;
	public:
		Opener(nlohmann::json& j);
		~Opener();

		//get next item from queue
		BWAPI::UnitType Next() { return _queue.front(); }

		//remove first element for queue
		//@return true if opener was finished
		bool Pop();
	};

}
