#include "DebugModule.h"
#include "Config.h"

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

DebugModule* DebugModule::Instance()
{
	if (!_instance)
		_instance = new DebugModule;
	return _instance;
}

void DebugModule::DrawDebug()
{
	//TODO draw stuff here
}
