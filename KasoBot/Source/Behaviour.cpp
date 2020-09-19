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
		//TODO scout enemy tech inside base
		//TODO scout natural timing
		unit.SetRole(Units::Role::IDLE);
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
