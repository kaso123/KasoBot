#include "ScoutModule.h"
#include "Config.h"
#include "MapModule.h"
#include "WorkersModule.h"
#include "ProductionModule.h"

#include "BaseInfo.h"

using namespace KasoBot;

ScoutModule* ScoutModule::_instance = 0;

ScoutModule::ScoutModule()
	: _enemyStart(nullptr)
{
}

ScoutModule::~ScoutModule()
{
	delete(_instance);
}

void ScoutModule::ResetEnemyInfo()
{
	if (BWAPI::Broodwar->getFrameCount() % Config::Units::EnemyPositionResetFrames() != 0)
		return;

	for (auto& type : _enemies)
	{
		for (auto& enemy : type.second)
		{
			if (enemy.hidden)
			{
				if (!enemy.type.isBuilding() && enemy.lastPos != BWAPI::TilePositions::Unknown)
				{
					//set position to unknown when time has elapsed
					if (BWAPI::Broodwar->getFrameCount() > enemy.lastSeenFrame + Config::Units::HiddenPositionResetFrames())
						enemy.lastPos = BWAPI::TilePositions::Unknown;
				}
				continue;
			}

			auto pointer = BWAPI::Broodwar->getUnit(enemy.id);

			if (!pointer)
				continue;

			auto pos = pointer->getTilePosition();
			if (!pos.isValid() || pos == BWAPI::TilePositions::Unknown)
				continue;

			enemy.lastPos = pos;
			enemy.lastSeenFrame = BWAPI::Broodwar->getFrameCount();
		}
	}
}

void ScoutModule::RemoveByID(int unitID, BWAPI::UnitType oldType)
{
	auto it_type = _enemies.find(oldType);

	if (it_type == _enemies.end())
		return;

	for (auto it = it_type->second.begin(); it != it_type->second.end(); it++)
	{
		if (unitID == it->id)
		{
			it_type->second.erase(it);
			return;
		}
	}
}

void ScoutModule::CheckEnemyEvolution(BWAPI::Unit unit)
{
	//if unit with this id was different type before -> remove it
	if (unit->getType() == BWAPI::UnitTypes::Zerg_Lurker)
		RemoveByID(unit->getID(), BWAPI::UnitTypes::Zerg_Hydralisk);
	else if (unit->getType() == BWAPI::UnitTypes::Zerg_Guardian)
		RemoveByID(unit->getID(), BWAPI::UnitTypes::Zerg_Mutalisk);
	else if (unit->getType() == BWAPI::UnitTypes::Zerg_Devourer)
		RemoveByID(unit->getID(), BWAPI::UnitTypes::Zerg_Mutalisk);
	else if (unit->getType() == BWAPI::UnitTypes::Zerg_Sunken_Colony)
		RemoveByID(unit->getID(), BWAPI::UnitTypes::Zerg_Creep_Colony);
	else if (unit->getType() == BWAPI::UnitTypes::Zerg_Spore_Colony)
		RemoveByID(unit->getID(), BWAPI::UnitTypes::Zerg_Creep_Colony);
	else if (unit->getType() == BWAPI::UnitTypes::Zerg_Greater_Spire)
		RemoveByID(unit->getID(), BWAPI::UnitTypes::Zerg_Spire);
	else if (unit->getType() == BWAPI::UnitTypes::Zerg_Lair)
		RemoveByID(unit->getID(), BWAPI::UnitTypes::Zerg_Hatchery);
	else if (unit->getType() == BWAPI::UnitTypes::Zerg_Hive)
	{
		RemoveByID(unit->getID(), BWAPI::UnitTypes::Zerg_Hatchery);
		RemoveByID(unit->getID(), BWAPI::UnitTypes::Zerg_Lair);
	}
	//protoss
	else if (unit->getType() == BWAPI::UnitTypes::Protoss_Archon)
		RemoveByID(unit->getID(), BWAPI::UnitTypes::Protoss_High_Templar);
	else if (unit->getType() == BWAPI::UnitTypes::Protoss_Dark_Archon)
		RemoveByID(unit->getID(), BWAPI::UnitTypes::Protoss_Dark_Templar);
}

ScoutModule* ScoutModule::Instance()
{
	if (!_instance)
		_instance = new ScoutModule;
	return _instance;
}

void ScoutModule::OnFrame()
{
	ResetEnemyInfo();

}

void ScoutModule::OnStart()
{
	Map::ResetBaseInfo(_baseInfo);
}

void ScoutModule::EnemyDiscovered(BWAPI::Unit unit)
{
	//skip our own and neutral units
	if (!unit->getPlayer() || !unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()))
		return;

	if (unit->getType() == BWAPI::UnitTypes::Zerg_Egg || unit->getType() == BWAPI::UnitTypes::Terran_Vulture_Spider_Mine 
		|| unit->getType() == BWAPI::UnitTypes::Spell_Scanner_Sweep || unit->getType() == BWAPI::UnitTypes::Zerg_Larva
		|| unit->getType() == BWAPI::UnitTypes::Zerg_Cocoon || unit->getType() == BWAPI::UnitTypes::Zerg_Lurker_Egg)
		return;

	//set enemy start if not found before
	if (!_enemyStart && unit->getType().isBuilding())
	{
		_enemyStart = Map::ClosestStart(unit->getTilePosition());

		//set base as belonging to enemy
		if (_enemyStart && !_enemyStart->Bases().empty())
		{
			((BaseInfo*)_enemyStart->Bases().front().Ptr())->_owner = Base::Owner::ENEMY;
		}
	}

	if (unit->getType().isBuilding()) //reset build items, so they don't try to build on enemy buildings
	{
		ProductionModule::Instance()->TileOccupied(unit);
	}

	auto it_type = _enemies.find(unit->getType()); //find type

	if (it_type != _enemies.end())
	{
		for (auto& enemy : it_type->second)
		{
			if (enemy.id == unit->getID()) //find this unit by ID
			{
				enemy.lastPos = BWAPI::TilePosition(unit->getPosition());
				enemy.hidden = false;
				return;
			}
		}
		//if not in the list, create new entry
		it_type->second.emplace_back(EnemyUnit(unit));

	}
	//if first enemy of this type add new vector to list
	else
	{
		auto new_it = _enemies.insert({ unit->getType(), EnemyList{} });
		new_it.first->second.emplace_back(EnemyUnit(unit));
	}

	CheckEnemyEvolution(unit);
}

void ScoutModule::EnemyHidden(BWAPI::Unit unit)
{
	for(auto& type : _enemies)
	{
		for (auto& enemy : type.second)
		{
			if (enemy.id == unit->getID()) //find this unit by ID
			{
				enemy.hidden = true;
				return;
			}
		}
	}
}

void ScoutModule::EnemyDestroyed(BWAPI::Unit unit)
{
	//skip our own and neutral units
	if (!unit->getPlayer() || !unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()))
		return;

	if (unit->getType() == BWAPI::UnitTypes::Zerg_Egg || unit->getType() == BWAPI::UnitTypes::Terran_Vulture_Spider_Mine
		|| unit->getType() == BWAPI::UnitTypes::Spell_Scanner_Sweep || unit->getType() == BWAPI::UnitTypes::Zerg_Larva
		|| unit->getType() == BWAPI::UnitTypes::Zerg_Cocoon || unit->getType() == BWAPI::UnitTypes::Zerg_Lurker_Egg)
		return;

	auto it_type = _enemies.find(unit->getType()); //find type

	if (it_type != _enemies.end())
	{
		for (auto it = it_type->second.begin(); it != it_type->second.end(); it++)
		{
			if ((*it).id == unit->getID()) //find this unit by ID
			{
				it_type->second.erase(it);
				break;
			}
		}
		if (it_type->second.empty())
			_enemies.erase(it_type);
	}
}

bool ScoutModule::ShouldWorkerScout()
{
	if (_enemyStart)
		return false;
	if (WorkersModule::Instance()->WorkerCountMinerals() + WorkersModule::Instance()->WorkerCountGas() + 1 < Config::Strategy::FirstScoutSupply())
		return false;

	return true;
}

