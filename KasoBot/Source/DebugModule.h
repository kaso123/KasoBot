#pragma once
#include <BWAPI.h>

namespace KasoBot {

	namespace Workers {
		enum Role;
	}
	namespace Units {
		enum Role;
	}

	class Worker;
	class Task;

	class DebugModule
	{
	private:
		DebugModule();
		~DebugModule();
		static DebugModule* _instance;

		bool _drawMap;
		bool _drawWorkers;
		bool _drawArmy;
		bool _drawTasks;
		bool _drawProduction;
		bool _drawStrategy;
		bool _drawOrders;
		bool _drawBases;
		bool _drawResources;
		bool _drawEnemy;

		void DrawMap();
		void DrawWorkers();
		void DrawSingleWorker(const Worker& worker);
		int DrawArmy();
		void DrawTasks();
		void DrawProduction();
		void DrawBases();
		void DrawResources();
		void DrawStrategy();
		void DrawEnemy(int y);

		//get selected units in GUI and make them player controlled
		void SwitchControlOnSelected();

		//@return string representation of worker role enum
		const char* WorkerRoleString(Workers::Role role);

		//@return string representation of worker role enum
		const char* UnitRoleString(Units::Role role);

		//@return string representation of task
		const char* GetTaskString(Task* task);
	
	public:
		static DebugModule* Instance();

		void DrawDebug();

		void DebugCommand(std::string& text);
	};
}

