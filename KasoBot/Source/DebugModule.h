#pragma once
#include <BWAPI.h>

namespace KasoBot {

	namespace Workers {
		enum Role;
	}

	class Worker;

	class DebugModule
	{
	private:
		DebugModule();
		~DebugModule();
		static DebugModule* _instance;

		bool _drawMap;
		bool _drawWorkers;
		bool _drawArmy;
		bool _drawProduction;
		bool _drawStrategy;
		bool _drawOrders;
		bool _drawBases;
		bool _drawResources;
		bool _drawEnemy;

		void DrawMap();
		void DrawWorkers();
		void DrawSingleWorker(const Worker& worker);
		void DrawArmy();
		void DrawProduction();
		void DrawBases();
		void DrawResources();
		void DrawStrategy();
		void DrawEnemy();

		//get selected units in GUI and make them player controlled
		void SwitchControlOnSelected();

		//@return string representation of worker role enum
		const char* WorkerRoleString(Workers::Role role);
	public:
		static DebugModule* Instance();

		void DrawDebug();

		void DebugCommand(std::string& text);
	};
}

