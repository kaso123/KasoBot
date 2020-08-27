#include "DebugModule.h"
#include "Config.h"
#include "MapModule.h"
#include "WorkersModule.h"
#include "Expansion.h"
#include "Worker.h"

using namespace KasoBot;

DebugModule* DebugModule::_instance = 0;

DebugModule::DebugModule()
	:_drawMap(Config::Debug::Map()), _drawWorkers(Config::Debug::Workers()), _drawArmy(Config::Debug::Army())
	, _drawBuildOrder(Config::Debug::BuildOrder()), _drawStrategy(Config::Debug::Strategy())
{
}

DebugModule::~DebugModule()
{
	delete(_instance);
}

void DebugModule::DrawMap()
{
	BWEB::Map::draw();
}

void DebugModule::DrawWorkers()
{
	for (const auto& exp : WorkersModule::Instance()->ExpansionList())
	{
		for (const auto worker : exp->Workers())
		{
			//draw worker role
			BWAPI::Broodwar->drawTextMap(worker->GetPointer()->getPosition(), WorkerRoleString(worker->GetWorkerRole()));

			//draw worker target
			if (worker->GetMineral())
				BWAPI::Broodwar->drawLineMap(worker->GetPointer()->getPosition(), worker->GetMineral()->Pos(), BWAPI::Colors::Blue);
			if (worker->GetRefinery())
				BWAPI::Broodwar->drawLineMap(worker->GetPointer()->getPosition(), worker->GetRefinery()->getPosition(), BWAPI::Colors::Green);
		}
	}
}

const char* DebugModule::WorkerRoleString(Workers::Role role)
{
	if (role == Workers::Role::MINERALS)
		return "\x1c Minerals"; //grey blue
	if (role == Workers::Role::GAS)
		return "\x1d Gas"; //grey green
	if (role == Workers::Role::ASSIGNED)
		return "\x03 Assigned"; //yellow
	if (role == Workers::Role::BUILDING)
		return "\x15 Scout"; //player white

		return "Idle";
}

DebugModule* DebugModule::Instance()
{
	if (!_instance)
		_instance = new DebugModule;
	return _instance;
}

void DebugModule::DrawDebug()
{
	//draw stuff here
	if (_drawMap)
		DrawMap();
	if (_drawWorkers)
		DrawWorkers();
}
