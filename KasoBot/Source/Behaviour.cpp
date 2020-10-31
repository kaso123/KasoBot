#include "Behaviour.h"
#include "Unit.h"
#include "MapModule.h"
#include "ScoutModule.h"
#include "BaseInfo.h"
#include "Config.h"
#include "Army.h"
#include "EnemyArmy.h"
#include "Task.h"
#include "ArmyModule.h"
#include "Log.h"

using namespace KasoBot;

void Behaviour::Move(BWAPI::Unit unit, BWAPI::Position position)
{
	if (unit->getOrder() == BWAPI::Orders::Move && unit->getOrderTargetPosition() == position)
		return;

	unit->move(position);
}

void Behaviour::AttackMove(BWAPI::Unit unit, BWAPI::Position position)
{
	if (unit->getOrder() == BWAPI::Orders::AttackMove && unit->getOrderTargetPosition().getDistance(position) < Config::Units::OrderDistSimilarity())
		return;

	//reset attack move when attacking target to stop chasing after enemies
	//keep attacking when the unit is close to amove position or when it is cannon rush
	if (unit->getOrder() == BWAPI::Orders::AttackUnit && 
		(unit->getOrderTarget() && (unit->getOrderTarget()->getType() == BWAPI::UnitTypes::Protoss_Pylon || unit->getOrderTarget()->getType() == BWAPI::UnitTypes::Protoss_Photon_Cannon)
			|| unit->getOrderTarget() && unit->getOrderTarget()->exists() && unit->getOrderTarget()->getDistance(position) < (Config::Units::EnemyArmyRange() * 32) 
			|| (unit->getLastCommandFrame() + 2 * Config::Units::OrderDelay()) > BWAPI::Broodwar->getFrameCount())) 
		return;

	unit->attack(position);
}

void Behaviour::HoldPosition(BWAPI::Unit unit)
{
	if (unit->getOrder() == BWAPI::Orders::HoldPosition)
		return;

	unit->holdPosition();
}

Behaviour::Behaviour()
{
}

Behaviour::~Behaviour()
{
}

void Behaviour::AttackArea(KasoBot::Unit & unit, Army* army)
{
	Log::Instance()->Assert(army,"No army in behaviour task!");
	auto area = army->Task()->Area();
	Log::Instance()->Assert(!area->Bases().empty(),"No bases in area in attack task!");
	
	for (auto & base : area->Bases())
	{
		if (((BaseInfo*)base.Ptr())->_owner == Base::Owner::ENEMY
			|| ((BaseInfo*)base.Ptr())->_owner == Base::Owner::UNKNOWN)
		{
			AttackMove(unit.GetPointer(),base.Center());
		}
	}
}

void Behaviour::ScoutArea(KasoBot::Unit & unit, Army * army)
{
	Log::Instance()->Assert(army, "No army in behaviour task!");
	auto area = army->Task()->Area();
	Log::Instance()->Assert(!area->Bases().empty(), "No bases in area in scout task!");

	for (auto & base : area->Bases())
	{
		if (((BaseInfo*)base.Ptr())->_owner == Base::Owner::UNKNOWN)
		{
			//if we are scouting with bigger army use a-move
			if (army->GetSupply() > 2) 
			{
				AttackMove(unit.GetPointer(), base.Center());
			}
			else Move(unit.GetPointer(), base.Center());
		}
	}
}

void Behaviour::DefendArmy(KasoBot::Unit& unit, Army* army)
{
	Log::Instance()->Assert(army, "No army in behaviour task!");
	auto enemyArmy = army->Task()->EnemyArmy();
	Log::Instance()->Assert(enemyArmy,"No enemy army in defend task!");
	
	if (ArmyModule::Instance()->Bunker() && !army->Task()->EnemyArmy()->IsCannonRush()) //stick to bunker if enemy is not inside our base
	{
		auto enemyPos = army->Task()->EnemyArmy()->BoundingBox()._center;
		auto bunker = ArmyModule::Instance()->Bunker()->GetPointer();
		if (bunker->getDistance(BWEB::Map::getMainPosition()) < enemyPos.getDistance(BWEB::Map::getMainPosition())
			&& bunker->getDistance(enemyPos) < 500) //TODO configurable
		{
			AttackMove(unit.GetPointer(), ArmyModule::Instance()->Bunker()->GetPointer()->getPosition());
			return;
		}
	}
	AttackMove(unit.GetPointer(), enemyArmy->BoundingBox()._center);
}

void Behaviour::HoldPosition(KasoBot::Unit & unit, Army * army)
{
	Log::Instance()->Assert(army, "No army in behaviour task!");
	auto pos = army->Task()->Position();
	Log::Instance()->Assert(pos.isValid(),"Invalid position in hold task!");

	if (unit.GetPointer()->getDistance(pos) > Config::Units::HoldPositionDistance())
		AttackMove(unit.GetPointer(), pos);
	else
		HoldPosition(unit.GetPointer());
}

void Behaviour::FinishEnemy(KasoBot::Unit & unit, Army * army)
{
	for (auto& type : ScoutModule::Instance()->GetEnemies())
	{
		for (auto& enemy : type.second)
		{
			if (enemy->_lastPos.isValid() && enemy->_lastPos != BWAPI::TilePositions::Unknown)
			{
				BWAPI::Position target = BWAPI::Position(enemy->_lastPos) + BWAPI::Position(16, 16); //center of tile
				AttackMove(unit.GetPointer(), target);
				return;
			}
		}
	}
	//no enemy in list
	auto pos = army->Task()->Next();
	Log::Instance()->Assert(pos.isValid(),"invalid position for finishig enemy!");
	AttackMove(unit.GetPointer(), BWAPI::Position(pos) + BWAPI::Position(16,16));
}

void Behaviour::MoveToArmyCenter(KasoBot::Unit & unit, BWAPI::Position position)
{
	Log::Instance()->Assert(position.isValid(),"Invalid position in MoveToArmyCenter!");

	AttackMove(unit.GetPointer(), position);
}

void Behaviour::Scout(KasoBot::Unit & unit)
{
	if (ScoutModule::Instance()->EnemyStart())
	{
		BWAPI::Position pos = BWAPI::Positions::Invalid;

		//scout enemy natural
		if (ScoutModule::Instance()->EnemyNatural()
			&& ((BaseInfo*)ScoutModule::Instance()->EnemyNatural()->Bases().front().Ptr())->_owner == Base::Owner::UNKNOWN)
		{
			pos = ScoutModule::Instance()->EnemyNatural()->Bases().front().Center();
		}
		else //scout enemy tech inside base
		{
			pos = Map::NextScoutPosition(ScoutModule::Instance()->EnemyStart(), unit.GetPointer()->getPosition());
		}

		if (!pos.isValid())
			return;

		//TODO this is debug drawing
		BWAPI::Broodwar->registerEvent([pos](BWAPI::Game*) { BWAPI::Broodwar->drawCircleMap(pos,5,BWAPI::Colors::Blue,true); },   // action
			nullptr,    // condition
			BWAPI::Broodwar->getLatencyFrames());  // frames to run

		Move(unit.GetPointer(), pos);

		return;
	}

	auto base = Map::NextScoutBaseStart();
	if (base)
	{
		//if close to base and didn't see an enemy building set it as free
		if (unit.GetPointer()->getDistance(base->Center()) <= Config::Workers::BuildStartDistance())
		{
			((BaseInfo*)base->Ptr())->_owner = Base::Owner::NONE;
			return;
		}

		//move to base
		Move(unit.GetPointer(), base->Center());
	}
}

void Behaviour::ScoutRush(KasoBot::Unit & unit)
{
	BWAPI::Position pos = BWAPI::Positions::Invalid;
	
	int mod = BWAPI::Broodwar->getFrameCount() % 1500;
	if (mod < 500) //every 1500 frames switch main<->natural
	{
		//main
		pos = Map::NextScoutPosition(BWEB::Map::getMainArea(), unit.GetPointer()->getPosition());
	}
	else if (mod < 1200)
	{
		//natural
		pos = Map::NextScoutPosition(BWEB::Map::getNaturalArea(), unit.GetPointer()->getPosition());
	}
	else
	{
		pos = (BWAPI::Position)BWEB::Map::getNaturalChoke()->Center();
	}

	if (!pos.isValid())
		return;

	//TODO this is debug drawing
	BWAPI::Broodwar->registerEvent([pos](BWAPI::Game*) { BWAPI::Broodwar->drawCircleMap(pos, 5, BWAPI::Colors::Blue, true); },   // action
		nullptr,    // condition
		BWAPI::Broodwar->getLatencyFrames());  // frames to run

	Move(unit.GetPointer(), pos);
}
