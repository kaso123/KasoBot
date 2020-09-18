#include "ScoutModule.h"
#include "Config.h"

using namespace KasoBot;

ScoutModule* ScoutModule::_instance = 0;

ScoutModule::ScoutModule()
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
				continue;
			auto pointer = BWAPI::Broodwar->getUnit(enemy.id);

			if (!pointer)
				continue;

			auto pos = pointer->getTilePosition();
			if (!pos.isValid() || pos == BWAPI::TilePositions::Unknown)
				continue;

			enemy.lastPos = pos;
		}
	}
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

void ScoutModule::EnemyDiscovered(BWAPI::Unit unit)
{
	//skip our own and neutral units
	if (!unit->getPlayer() || !unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()))
		return;

	if (unit->getType() == BWAPI::UnitTypes::Zerg_Egg || unit->getType() == BWAPI::UnitTypes::Terran_Vulture_Spider_Mine 
		|| unit->getType() == BWAPI::UnitTypes::Spell_Scanner_Sweep || unit->getType() == BWAPI::UnitTypes::Zerg_Larva)
		return;

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
		|| unit->getType() == BWAPI::UnitTypes::Spell_Scanner_Sweep || unit->getType() == BWAPI::UnitTypes::Zerg_Larva)
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
