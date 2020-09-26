#include "Behaviour.h"
#include "Unit.h"
#include "MapModule.h"
#include "ScoutModule.h"
#include "BaseInfo.h"
#include "Config.h"

using namespace KasoBot;

void Behaviour::Move(BWAPI::Unit unit, BWAPI::Position position)
{
	if (unit->getOrder() == BWAPI::Orders::Move && unit->getOrderTargetPosition() == position)
		return;

	unit->move(position);
}

Behaviour::Behaviour()
{
}

Behaviour::~Behaviour()
{
}

void Behaviour::Scout(KasoBot::Unit & unit)
{
	if (ScoutModule::Instance()->EnemyStart())
	{
		//scout enemy tech inside base
		auto pos = Map::NextScoutPosition(ScoutModule::Instance()->EnemyStart(), unit.GetPointer()->getPosition());
		if (!pos.isValid())
			return;

		//TODO this is debug drawing
		BWAPI::Broodwar->registerEvent([pos](BWAPI::Game*) { BWAPI::Broodwar->drawCircleMap(pos,5,BWAPI::Colors::Blue,true); },   // action
			nullptr,    // condition
			BWAPI::Broodwar->getLatencyFrames());  // frames to run

		Move(unit.GetPointer(), pos);

		//TODO scout natural timing
		return;
	}

	auto base = Map::NextScoutBase();
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
