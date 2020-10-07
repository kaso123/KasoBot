#pragma once
#include <BWAPI.h>

namespace BWEM {
	class Area;
}

namespace KasoBot {

	namespace Tasks {
		enum Type {
			ATTACK,
			DEFEND,
			SCOUT
		};
	}
	class Task
	{
	private:
		Tasks::Type _type;
		BWAPI::Position _pos;
		const BWEM::Area* _area;
		bool _inProgress;

	public:
		Task(Tasks::Type type, BWAPI::Position pos);
		Task(Tasks::Type type, const BWEM::Area* area);
		~Task();

		//getters and setters

		void Start() { _inProgress = true; }
		void Stop() { _inProgress = false; }
		Tasks::Type Type() { return _type; }
		BWAPI::Position Position() { return _pos; }
		const BWEM::Area* Area() { return _area; }
		bool InProgress() { return _inProgress; }
	};
}
