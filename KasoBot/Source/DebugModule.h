#pragma once
#include <BWAPI.h>

namespace KasoBot {

	namespace Workers {
		enum Role;
	}

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

		void DrawMap();
		void DrawWorkers();

		//@return string representation of worker role enum
		const char* WorkerRoleString(Workers::Role role);
	public:
		static DebugModule* Instance();

		void DrawDebug();
	};
}

