#include "Task.h"
#include "MapModule.h"

using namespace KasoBot;


Task::Task(Tasks::Type type, BWAPI::Position pos)
	:_type(type), _pos(pos), _area(nullptr), _inProgress(false)
{
}

Task::Task(Tasks::Type type, const BWEM::Area * area)
	: _type(type), _area(area), _pos(BWAPI::Positions::Invalid), _inProgress(false)
{
}

Task::~Task()
{
}
