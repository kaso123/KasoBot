#pragma once
#include <BWAPI.h>

namespace KasoBot {

	class DebugModule
	{
	private:
		DebugModule();
		~DebugModule();
		static DebugModule* _instance;

		bool _drawMap;
		bool _drawWorkers;
		bool _drawArmy;
		bool _drawBuildOrder;
		bool _drawStrategy;
	public:
		static DebugModule* Instance();

		void DrawDebug();
	};
}

