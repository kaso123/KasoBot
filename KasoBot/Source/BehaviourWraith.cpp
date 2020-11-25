#include "BehaviourWraith.h"
#include "Army.h"
#include "Unit.h"
#include "Task.h"
#include "MapModule.h"
#include "Log.h"
#include "BaseInfo.h"
#include "MapModule.h"

#define HARASS_START_DIST 100

using namespace KasoBot;


BehaviourWraith::BehaviourWraith()
{
}

BehaviourWraith::~BehaviourWraith()
{
}

void BehaviourWraith::HarassArea(KasoBot::Unit & unit, Army * army)
{
	Log::Instance()->Assert(army, "No army in behaviour!");
	auto area = army->Task()->Area();
	Log::Instance()->Assert(!area->Bases().empty(), "No bases in area in harass task!");

	bool cp = army->Task()->IsCheckpointDone();

	for (auto & base : area->Bases())
	{
		if (((BaseInfo*)base.Ptr())->_owner == Base::Owner::ENEMY
			|| ((BaseInfo*)base.Ptr())->_owner == Base::Owner::UNKNOWN)
		{
			if (cp) //move to harass base
			{
				BWAPI::Position pos = Map::GetHarassContactPoint(&base);
				Log::Instance()->Assert(pos.isValid(), "Invalid contact point for base harassment!");

				//attack move if close
				if (unit.GetPointer()->getDistance(pos) < HARASS_START_DIST)
				{
					AttackMove(unit.GetPointer(), pos, true);
					return;
				}
				else //move if not in position
				{
					Move(unit.GetPointer(), pos);
					return;
				}				
			}
			else //move to cp
			{
				BWAPI::Position pos = Map::GetHarassCheckpoint(&base);
				Log::Instance()->Assert(pos.isValid(), "Invalid checkpoint for base harassment!");

				if (unit.GetPointer()->getDistance(pos) < HARASS_START_DIST)
					army->Task()->SetCheckpointDone();
				Move(unit.GetPointer(), pos);
				return;
			}
			return;
		}
	}
}