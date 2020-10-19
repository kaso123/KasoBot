#include "EnemyArmy.h"
#include "ScoutModule.h"
#include "Army.h"
#include "ArmyModule.h"
#include "Config.h"
#include "ProductionModule.h"
#include "WorkersModule.h"
#include "Unit.h"
#include "Expansion.h"

using namespace KasoBot;

void EnemyArmy::CalculateCenter()
{
	int minX = INT_MAX;
	int minY = INT_MAX;
	int maxX = INT_MIN;
	int maxY = INT_MIN;

	_ASSERT(!_units.empty());

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
	for (auto unit : _units)
	{
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
	_ASSERT(_units.empty());
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

	_ASSERT(false); //called from enemy destructor so the unit has to be in this army
}

void EnemyArmy::Join(EnemyArmy * toJoin)
{
	for (auto& unit : toJoin->Units())
	{
		AddEnemy(unit);
	}
	toJoin->ClearUnits();
}

void EnemyArmy::ClearUnits()
{
	_units.clear();
}

bool EnemyArmy::IsThreat()
{
	for (auto& type : ProductionModule::Instance()->Buildings())
	{
		_ASSERT(type.first.isBuilding());

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

	for (auto& enemy : _units)
	{
		if (enemy->_type == BWAPI::UnitTypes::Protoss_Photon_Cannon
			|| enemy->_type == BWAPI::UnitTypes::Protoss_Pylon)
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

int EnemyArmy::Supply()
{
	int total = 0;
	for (auto& unit : _units)
	{
		if (unit->_type == BWAPI::UnitTypes::Protoss_Photon_Cannon)
			total += 4;
		if (unit->_type == BWAPI::UnitTypes::Protoss_Pylon)
			total += 2;

		total += unit->_type.supplyRequired();
	}
	return total;
}
