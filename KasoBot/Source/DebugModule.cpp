#include "DebugModule.h"
#include "Config.h"
#include "MapModule.h"
#include "WorkersModule.h"
#include "ProductionModule.h"
#include "StrategyModule.h"
#include "ArmyModule.h"
#include "ScoutModule.h"

#include "Expansion.h"
#include "Worker.h"
#include "ProductionItem.h"
#include "Army.h"
#include "BaseInfo.h"
#include "EnemyStrategy.h"

using namespace KasoBot;

DebugModule* DebugModule::_instance = 0;

DebugModule::DebugModule()
	:_drawMap(Config::Debug::Map()), _drawWorkers(Config::Debug::Workers()), _drawArmy(Config::Debug::Army())
	, _drawProduction(Config::Debug::BuildOrder()), _drawStrategy(Config::Debug::Strategy()), _drawOrders(Config::Debug::Orders())
	, _drawResources(Config::Debug::Resources()), _drawEnemy(Config::Debug::Enemy()), _drawBases(Config::Debug::Bases())
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
		for (const auto& worker : exp->Workers())
		{
			DrawSingleWorker(*worker);
		}
	}

	for (const auto& worker : WorkersModule::Instance()->Builders())
	{
		DrawSingleWorker(*worker);
	}
}

void DebugModule::DrawSingleWorker(const Worker& worker)
{
	//draw worker role
	BWAPI::Broodwar->drawTextMap(worker.GetPointer()->getPosition(), WorkerRoleString(worker.GetWorkerRole()));

	//draw worker target
	if (worker.GetProductionItem())
	{
		BWAPI::Broodwar->drawLineMap(worker.GetPointer()->getPosition(), Map::GetCenterOfBuilding(worker.GetProductionItem()->GetLocation(), worker.GetProductionItem()->GetType()), BWAPI::Colors::Yellow);
		BWAPI::Broodwar->drawTextMap(BWAPI::Position(worker.GetProductionItem()->GetLocation()), worker.GetProductionItem()->GetType().getName().c_str());
	}
		if (worker.GetMineral())
		BWAPI::Broodwar->drawLineMap(worker.GetPointer()->getPosition(), worker.GetMineral()->Pos(), BWAPI::Colors::Blue);
	if (worker.GetRefinery())
		BWAPI::Broodwar->drawLineMap(worker.GetPointer()->getPosition(), worker.GetRefinery()->getPosition(), BWAPI::Colors::Green);

	//draw worker game order
	if (_drawOrders)
	{
		BWAPI::Broodwar->drawTextMap(worker.GetPointer()->getPosition() + BWAPI::Position(25, 10), worker.GetPointer()->getOrder().getName().c_str());
		BWAPI::Broodwar->drawTextMap(worker.GetPointer()->getPosition() + BWAPI::Position(0, 10), "%i", std::min(BWAPI::Broodwar->getFrameCount() - worker.GetPointer()->getLastCommandFrame(), 999));
	}

	//draw if player controlled
	if (worker.PlayerControlled())
		BWAPI::Broodwar->drawCircleMap(worker.GetPointer()->getPosition(), 10, BWAPI::Colors::Red);
}

int DebugModule::DrawArmy()
{
	for (auto& worker : ArmyModule::Instance()->Workers())
	{
		//draw worker role
		BWAPI::Broodwar->drawTextMap(worker->GetPointer()->getPosition(), UnitRoleString(worker->GetRole()));
	}

	int y = 15;
	int i = 1;
	for (auto& army : ArmyModule::Instance()->Armies())
	{
		BWAPI::Broodwar->drawTextScreen(420, y, "Army no. %i: %i supply", i++, army->GetSupply() / 2);
		y += 10;
	}
	return y+5;
}

void DebugModule::DrawProduction()
{
	int y = 45;
	char color = '\x02';

	for (auto& item : ProductionModule::Instance()->GetItems())
	{
		if (item->GetState() == Production::State::ASSIGNED) color = '\x11'; //orange
		if (item->GetState() == Production::State::BUILDING) color = '\x07'; //green
		if (item->GetState() == Production::State::WAITING) color = '\x02'; //default
		if (item->GetState() == Production::State::UNFINISHED) color = '\x03'; //TODO what color is this?
		if (item->GetState() == Production::State::DONE) color = '\x1c'; //light blue

		BWAPI::Broodwar->drawTextScreen(10, y, "%c %s", color, item->GetType().getName().c_str());
		y += 10;
	}

	//draw locked buildings
	for (auto& type : ProductionModule::Instance()->Buildings())
	{
		for (auto& building : type.second)
		{
			if (building->IsLocked())
				BWAPI::Broodwar->drawLineMap(BWAPI::Position(building->GetPointer()->getTilePosition()),
					BWAPI::Position(building->GetPointer()->getTilePosition()) + BWAPI::Position(building->GetPointer()->getType().width(), building->GetPointer()->getType().height()),
					BWAPI::Colors::Red);
		}
	}

	//draw next unit type for all macro production types
	y += 10;
	//production
	BWAPI::Broodwar->drawTextScreen(10, y, "Next:\n%s", StrategyModule::Instance()->GetMacroProductionType().getName().c_str());
	
	y += 30;
	//army
	for (auto& type : StrategyModule::Instance()->GetMacroArmyTypes())
	{
		BWAPI::Broodwar->drawTextScreen(10, y, "%s", type.getName().c_str());
		y += 10;
	}

	y += 10;
	//tech/upgrades
	auto macro = StrategyModule::Instance()->GetMacroTechType();
	if (macro._unit != BWAPI::UnitTypes::None)
	{
		BWAPI::Broodwar->drawTextScreen(10, y, "%s", macro._unit.getName().c_str());
	}
	else if (macro._upgrade != BWAPI::UpgradeTypes::None)
	{
		BWAPI::Broodwar->drawTextScreen(10, y, "%s", macro._upgrade.getName().c_str());
	}
	else if (macro._tech != BWAPI::TechTypes::None)
	{
		BWAPI::Broodwar->drawTextScreen(10, y, "%s", macro._tech.getName().c_str());
	}
}

void DebugModule::DrawBases()
{
	for (const auto& exp : WorkersModule::Instance()->ExpansionList())
	{
		BWAPI::Broodwar->drawTextMap(BWAPI::Position(exp->GetStation()->getBWEMBase()->Location()).x, BWAPI::Position(exp->GetStation()->getBWEMBase()->Location()).y,
			"Min: %i/%i\nGas: %i/%i",
			exp->WorkerCountMinerals(),exp->GetStation()->getBWEMBase()->Minerals().size() * Config::Workers::SaturationPerMineral(),
			exp->WorkerCountGas(),exp->GetRefinery() ? Config::Workers::SaturationPerGas() : 0);
	}

	for (const auto& area : BWEM::Map::Instance().Areas())
	{
		for (const auto& base : area.Bases())
		{
			auto owner = ((BaseInfo*)base.Ptr())->_owner;
			std::string text;

			if (owner == Base::Owner::PLAYER)
				text = "Player";
			else if (owner == Base::Owner::ENEMY)
				text = "Enemy";
			else if (owner == Base::Owner::NONE)
				text = "Empty";
			else
				text = "Unknown";

			BWAPI::Broodwar->drawTextMap(base.Center(), "%s", text.c_str());
		}
	}
}

void DebugModule::DrawResources()
{
	//draw reserved resources
	BWAPI::Broodwar->drawTextScreen(10, 10, "Reserved:\x1c %i\x07 %i", ProductionModule::Instance()->GetReservedMinerals(), ProductionModule::Instance()->GetReservedGas());
	
	//draw enemy lost resources
	BWAPI::Broodwar->drawTextScreen(10, 20, "Enemy Lost:\x1c %i\x07 %i", StrategyModule::Instance()->EnemyLostMinerals(), StrategyModule::Instance()->EnemyLostGas());

}

void DebugModule::DrawStrategy()
{	
	//draw opener
	BWAPI::Broodwar->drawTextScreen(120, 10, "Opening: %s", StrategyModule::Instance()->GetOpenerName().c_str());
	BWAPI::Broodwar->drawTextScreen(120, 20, "Strat: %s", StrategyModule::Instance()->GetStratName().c_str());
	BWAPI::Broodwar->drawTextScreen(120, 35, "Enemy is: %s", ScoutModule::Instance()->GetEnemyRace().toString().c_str());
	
	int y = 45;
	for (auto& strat : StrategyModule::Instance()->GetEnemyStrategies())
	{
		BWAPI::Broodwar->drawTextScreen(120, y, "%s : %i", strat->GetName().c_str(),strat->Score());
		y += 10;
	}
}

void DebugModule::DrawEnemy(int y)
{
	//draw enemy counts for unit types
	BWAPI::Broodwar->drawTextScreen(420, y, "Enemies:");
	y += 10;

	for (auto& type : ScoutModule::Instance()->GetEnemies())
	{
		BWAPI::Broodwar->drawTextScreen(420, y, "%i %s", type.second.size(), type.first.getName().c_str());
		y += 10;
	}

	//draw positions around the map
	for (auto& type : ScoutModule::Instance()->GetEnemies())
	{
		for (auto& unit : type.second)
		{
			if (unit.hidden && unit.lastPos != BWAPI::TilePositions::Unknown) //only draw unit positions of units in fog-of-war
			{
				BWAPI::Broodwar->drawBoxMap(BWAPI::Position(unit.lastPos),
					BWAPI::Position(unit.lastPos) + BWAPI::Position(BWAPI::TilePosition(unit.type.tileWidth(),
						unit.type.tileHeight())), BWAPI::Colors::Red, false);
			}	
		}
	}
}

void DebugModule::SwitchControlOnSelected()
{
	auto selected = BWAPI::Broodwar->getSelectedUnits();

	//basic workers
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

	//builders
	for (auto& worker : WorkersModule::Instance()->Builders())
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

	//units
	for (auto& type : ProductionModule::Instance()->Units())
	{
		for (auto& unit : type.second)
		{
			for (auto sel : selected)
			{
				if (sel == unit->GetPointer())
				{
					unit->ChangeDebugControl();
					break;
				}
			}
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
	if (role == Workers::Role::BUILD)
		return "\x15 Build"; //player white

		return "Idle";
}

const char* DebugModule::UnitRoleString(Units::Role role)
{
	if (role == Units::Role::SCOUT)
		return "\x01 Scout"; //TODO color?

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
	if (_drawProduction)
		DrawProduction();
	if (_drawBases)
		DrawBases();
	if (_drawResources)
		DrawResources();
	if (_drawStrategy)
		DrawStrategy();

	int y = 15; //stop overlaping text
	if (_drawArmy)
	{
		y = DrawArmy();
	}
	if (_drawEnemy)
		DrawEnemy(y);
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
	else if (text == "bb")
	{
		ProductionModule::Instance()->DebugBuild(BWAPI::UnitTypes::Terran_Barracks);
	}
	else if (text == "bf")
	{
		ProductionModule::Instance()->DebugBuild(BWAPI::UnitTypes::Terran_Factory);
	}
	else if (text == "be")
	{
		ProductionModule::Instance()->DebugBuild(BWAPI::UnitTypes::Terran_Engineering_Bay);
	}
	else if (text == "bt")
	{
		ProductionModule::Instance()->DebugBuild(BWAPI::UnitTypes::Terran_Missile_Turret);
	}
	else if (text == "bcs")
	{
		ProductionModule::Instance()->DebugBuild(BWAPI::UnitTypes::Terran_Comsat_Station);
	}
	else if (text == "bms")
	{
		ProductionModule::Instance()->DebugBuild(BWAPI::UnitTypes::Terran_Machine_Shop);
	}
	else if (text == "ba")
	{
		ProductionModule::Instance()->DebugBuild(BWAPI::UnitTypes::Terran_Academy);
	}
}
