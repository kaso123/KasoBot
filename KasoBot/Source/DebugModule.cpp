#include "DebugModule.h"
#include "Config.h"
#include "MapModule.h"
#include "WorkersModule.h"
#include "ProductionModule.h"

#include "Expansion.h"
#include "Worker.h"
#include "ProductionItem.h"

using namespace KasoBot;

DebugModule* DebugModule::_instance = 0;

DebugModule::DebugModule()
	:_drawMap(Config::Debug::Map()), _drawWorkers(Config::Debug::Workers()), _drawArmy(Config::Debug::Army())
	, _drawBuildOrder(Config::Debug::BuildOrder()), _drawStrategy(Config::Debug::Strategy()), _drawOrders(Config::Debug::Orders())
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
			if (worker->GetProductionItem())
				BWAPI::Broodwar->drawLineMap(worker->GetPointer()->getPosition(), BWAPI::Position(worker->GetProductionItem()->GetLocation()), BWAPI::Colors::Yellow);
			if (worker->GetMineral())
				BWAPI::Broodwar->drawLineMap(worker->GetPointer()->getPosition(), worker->GetMineral()->Pos(), BWAPI::Colors::Blue);
			if (worker->GetRefinery())
				BWAPI::Broodwar->drawLineMap(worker->GetPointer()->getPosition(), worker->GetRefinery()->getPosition(), BWAPI::Colors::Green);

			//draw worker game order
			if (_drawOrders)
			{
				BWAPI::Broodwar->drawTextMap(worker->GetPointer()->getPosition() + BWAPI::Position(25, 10), worker->GetPointer()->getOrder().getName().c_str());
				BWAPI::Broodwar->drawTextMap(worker->GetPointer()->getPosition() + BWAPI::Position(0, 10), "%i", std::min(BWAPI::Broodwar->getFrameCount() - worker->GetPointer()->getLastCommandFrame(),999));
			}

			//draw if player controlled
			if (worker->PlayerControlled())
				BWAPI::Broodwar->drawCircleMap(worker->GetPointer()->getPosition(), 10, BWAPI::Colors::Red);	
		}
	}
}

void DebugModule::SwitchControlOnSelected()
{
	auto selected = BWAPI::Broodwar->getSelectedUnits();

	for (auto& exp : WorkersModule::Instance()->ExpansionList())
	{
		for (auto& worker : exp->Workers())
		{
			for (auto sel : selected)
			{
				if (sel == worker->GetPointer())
				{
					worker->ChangeDebugControl();
					break;
				}
			}
		}
	}

	//TODO do this for other units
}

const char* DebugModule::WorkerRoleString(Workers::Role role)
{
	if (role == Workers::Role::MINERALS)
		return "\x1c Minerals"; //grey blue
	if (role == Workers::Role::GAS)
		return "\x1d Gas"; //grey green
	if (role == Workers::Role::ASSIGNED)
		return "\x03 Assigned"; //yellow
	if (role == Workers::Role::BUILD)
		return "\x15 Build"; //player white

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
	//game speed changes with keys
	if (BWAPI::Broodwar->getKeyState(BWAPI::Key::K_G))
		BWAPI::Broodwar->setLocalSpeed(0);
	if (BWAPI::Broodwar->getKeyState(BWAPI::Key::K_H))
		BWAPI::Broodwar->setLocalSpeed(12);
	if (BWAPI::Broodwar->getKeyState(BWAPI::Key::K_J))
		BWAPI::Broodwar->setLocalSpeed(24);
	if (BWAPI::Broodwar->getKeyState(BWAPI::Key::K_K))
		BWAPI::Broodwar->setLocalSpeed(250);
	if (BWAPI::Broodwar->getKeyState(BWAPI::Key::K_L))
		BWAPI::Broodwar->setLocalSpeed(1500);

	//switch selected units from AI controlled to player controlled
	if (BWAPI::Broodwar->getKeyState(BWAPI::Key::K_N))
		SwitchControlOnSelected();

	//draw stuff here
	if (_drawMap)
		DrawMap();
	if (_drawWorkers)
		DrawWorkers();
}

void DebugModule::DebugCommand(std::string& text)
{
	if (text == "bc")
	{
		ProductionModule::Instance()->DebugBuild(BWAPI::UnitTypes::Terran_Command_Center);
	}
	else if (text == "br")
	{
		ProductionModule::Instance()->DebugBuild(BWAPI::UnitTypes::Terran_Refinery);
	}
	else if (text == "bs")
	{
		ProductionModule::Instance()->DebugBuild(BWAPI::UnitTypes::Terran_Supply_Depot);
	}
}
