#pragma once

#include <BWAPI.h>

namespace KasoBot {

	namespace Base {
		enum Owner {
			UNKNOWN,
			NONE,
			PLAYER,
			ENEMY
		};
	}

	struct BaseInfo {
		Base::Owner _owner = Base::Owner::UNKNOWN;
		int _lastSeenFrame = 0;
	};
}

