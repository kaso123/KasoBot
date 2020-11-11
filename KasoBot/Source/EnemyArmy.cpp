#include "EnemyArmy.h"
#include "ScoutModule.h"
#include "Army.h"
#include "ArmyModule.h"
#include "Config.h"
#include "ProductionModule.h"
#include "WorkersModule.h"
#include "Unit.h"
#include "Expansion.h"
#include "Log.h"

using namespace KasoBot;

void EnemyArmy::CalculateCenter()
{
	int minX = INT_MAX;
	int minY = INT_MAX;
	int maxX = INT_MIN;
	int maxY = INT_MIN;

	Log::Instance()->Assert(!_units.empty(),"No units in enemy army!");

	for (auto unit : _units)
	{
		auto pos = BWAPI::Position(unit->_lastPos);
		if (pos.x < minX) minX = pos.x;
		if (pos.x > maxX) maxX = pos.x;
		if (pos.y < minY) minY = pos.y;
		if (pos.y > maxY) maxY = pos.y;
	}

	_box->_topLeft = BWAPI::Position(minX, minY);
	_box->_bottomRight = BWAPI::Position(maxX + 32, maxY + 32); //draw also around last tile
	_box->_center = BWAPI::Position(minX + (maxX - minX) / 2, minY + (maxY - minY) / 2);
}

void EnemyArmy::CheckUnits()
{
	std::vector<EnemyUnit*> toRemove;
	for (auto& unit : _units)
	{
		if (unit->_type.isBuilding() && IsCannonRush())
			continue;

		if (unit->_lastPos == BWAPI::TilePositions::Unknown)
		{
			toRemove.emplace_back(unit);
			continue;
		}
		if (unit->_lastPos.getDistance(BWAPI::TilePosition(_box->_center)) > Config::Units::EnemyArmyRange())
		{
			toRemove.emplace_back(unit);
			continue;
		}
	}

	_units.erase(std::remove_if(_units.begin(), _units.end(), //remove units from this army
		[toRemove](auto& x)
		{
			for (auto item : toRemove)
			{
				if (item == x)
				{
					item->_army = nullptr;
					return true;
				}
			}
			return false;
		}
	), _units.end());
}

EnemyArmy::EnemyArmy()
{
	_box = std::make_unique<Armies::Box>();
	_box->_topLeft = BWAPI::Position(0, 0);
	_box->_bottomRight = BWAPI::Position(0, 0);
	_box->_center = BWAPI::Position(0, 0);
}

EnemyArmy::~EnemyArmy()
{
	Log::Instance()->Assert(_units.empty(),"Enemy army not empty in destructor!");
	ArmyModule::Instance()->EnemyArmyRemoved(this);
}

void EnemyArmy::OnFrame()
{
	CalculateCenter();
	CheckUnits();
}

void EnemyArmy::AddEnemy(EnemyUnit * unit)
{
	_units.emplace_back(unit);
	unit->_army = this;
	if (_units.size() == 1) //calculate initial center of army, later only calculate once per frame
		CalculateCenter();
}

void EnemyArmy::RemoveEnemy(EnemyUnit * unit)
{
	for (auto it = _units.begin(); it != _units.end(); it++)
	{
		if (*it == unit)
		{
			_units.erase(it);
			unit->_army = nullptr;
			return;
		}
	}

	Log::Instance()->Assert(false,"Enemy not found in army when deleting!"); //called from enemy destructor so the unit has to be in this army
}

bool EnemyArmy::Join(EnemyArmy * toJoin)
{
	if (IsCannonRush() || toJoin->IsCannonRush())
		return false;

	for (auto& unit : toJoin->Units())
	{
		AddEnemy(unit);
	}
	toJoin->ClearUnits();

	return true;
}

void EnemyArmy::ClearUnits()
{
	_units.clear();
}

bool EnemyArmy::IsThreat()
{
	if (IsWorkerRush())
	{
		for (auto& unit : _units)
		{
			auto pointer = BWAPI::Broodwar->getUnit(unit->_id);
			if (pointer && pointer->exists() && pointer->isVisible() && (pointer->isAttacking() || pointer->getGroundWeaponCooldown() > 0))
				return true;
		}
		return false;
	}

	if (IsCannonRush())
		return true;

	for (auto& type : ProductionModule::Instance()->Buildings())
	{
		Log::Instance()->Assert(type.first.isBuilding(),"Wrong type in building list!");

		for (auto& unit : type.second)
		{
			if (unit->GetPointer()->getDistance(_box->_center) < Config::Units::EnemyThreatRadius())
				return true;
		}
	}
	//also check expansions (they are not in building list)
	for (auto& exp : WorkersModule::Instance()->ExpansionList())
	{
		if (exp->GetPointer()->getDistance(_box->_center) < Config::Units::EnemyThreatRadius())
			return true;
	}


	return false;
}

bool EnemyArmy::IsCannonRush()
{
	for (auto& unit : _units)
	{
		if (unit->_type == BWAPI::UnitTypes::Protoss_Photon_Cannon
			|| unit->_type == BWAPI::UnitTypes::Protoss_Pylon)
			return true;
	}
	return  false;
}

bool EnemyArmy::IsWorkerRush()
{
	int workersInBase = 0;

	for (auto& unit : _units)
	{
		if (unit->_type.isWorker() && unit->_lastPos.isValid()
			&& unit->_lastPos.getDistance(BWAPI::Broodwar->self()->getStartLocation()) < 500)
			workersInBase++;

		if (!unit->_type.isWorker())
			workersInBase--;
	}
	return workersInBase > 1;
}

bool EnemyArmy::IsOnlyFlying()
{
	for (auto& enemy : _units)
	{
		if (!enemy->_type.isFlyer())
			return false;
	}
	return true;
}

int EnemyArmy::Supply()
{
	int total = 0;
	for (auto& unit : _units)
	{
		if (unit->_type == BWAPI::UnitTypes::Protoss_Photon_Cannon)
			total += 6;
		if (unit->_type == BWAPI::UnitTypes::Protoss_Pylon)
			total += 4;

		total += unit->_type.supplyRequired();
	}
	return total;
}
