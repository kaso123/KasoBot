#include "WorkersModule.h"
#include "Expansion.h"
#include "libs/BWEB/BWEB.h"

using namespace KasoBot;

WorkersModule* WorkersModule::_instance = 0;

WorkersModule::WorkersModule()
{
}

WorkersModule::~WorkersModule()
{
	delete(_instance);
}

WorkersModule* WorkersModule::Instance()
{
	if (!_instance)
		_instance = new WorkersModule;
	return _instance;
}

void WorkersModule::OnStart()
{

}

void WorkersModule::OnFrame()
{

}

void WorkersModule::NewWorker(BWAPI::Unit unit)
{
	//TODO select expansion for worker
}

void WorkersModule::RemoveWorker(BWAPI::Unit unit)
{
}

void WorkersModule::ExpansionCreated(BWAPI::Unit unit)
{
	_expansionList.emplace_back(std::make_shared<Expansion>(unit));
}

void WorkersModule::ExpansionDestroyed(BWAPI::Unit unit)
{
}
