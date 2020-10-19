#include "ScoutModule.h"
#include "Config.h"
#include "MapModule.h"
#include "WorkersModule.h"
#include "ProductionModule.h"
#include "ArmyModule.h"

#include "EnemyArmy.h"
#include "Army.h"
#include "BaseInfo.h"

using namespace KasoBot;

ScoutModule* ScoutModule::_instance = 0;


EnemyUnit::~EnemyUnit()
{
	if (_army)
		_army->RemoveEnemy(this);
}

ScoutModule::ScoutModule()
	: _enemyStart(nullptr), _enemyNatural(nullptr), _enemyRace(BWAPI::Races::Unknown)
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
			if (enemy->_hidden)
			{
				if (!enemy->_type.isBuilding() && enemy->_lastPos != BWAPI::TilePositions::Unknown)
				{
					//set position to unknown when time has elapsed
					if (BWAPI::Broodwar->getFrameCount() > enemy->_lastSeenFrame + Config::Units::HiddenPositionResetFrames())
						enemy->_lastPos = BWAPI::TilePositions::Unknown;
				}
				continue;
			}

			auto pointer = BWAPI::Broodwar->getUnit(enemy->_id);

			if (!pointer)
				continue;

			auto pos = pointer->getTilePosition();
			if (!pos.isValid() || pos == BWAPI::TilePositions::Unknown)
				continue;

			enemy->_lastPos = pos;
			enemy->_lastSeenFrame = BWAPI::Broodwar->getFrameCount();

			if (!enemy->_army)
				AssignToArmy(enemy.get());
		}
	}

	for (auto& type : _enemies) //remove buildings that are not there
	{
		if (!type.first.isBuilding())
			continue;

		type.second.erase(std::remove_if(type.second.begin(), type.second.end(),
			[](auto& x)
			{
				return !Map::IsStillThere(*x);
			}
		), type.second.end());
	}
	
	//remove empty types
	for (auto it = std::begin(_enemies); it != std::end(_enemies);)
	{
		if (it->second.empty())
		{
			it = _enemies.erase(it); 
		}
		else
			++it;
	}


	_armies.erase(std::remove_if(_armies.begin(), _armies.end(), //remove armies without units
		[](auto& x)
		{
			return x->Units().empty();
		}
	), _armies.end());
}

void ScoutModule::ResetBaseInfo()
{
	for (auto& station : BWEB::Stations::getStations())
	{
		auto base = station.getBWEMBase();
		BaseInfo* info = (BaseInfo*)base->Ptr();
		_ASSERT(info);

		if (info->_owner == Base::Owner::UNKNOWN)
		{
			if (Map::IsVisible(base))
			{
				info->_owner = Base::Owner::NONE;
				info->_lastSeenFrame = BWAPI::Broodwar->getFrameCount();
			}
			continue;
		}
		if (info->_owner == Base::Owner::NONE)
		{
			if (Map::IsVisible(base))
			{
				info->_lastSeenFrame = BWAPI::Broodwar->getFrameCount();
			}
			else
			{
				if (!_enemyStart) //don't reset state until we find enemy base
					break;

				if (info->_lastSeenFrame + Config::Units::HiddenBaseResetFrames() < BWAPI::Broodwar->getFrameCount())
				{
					info->_owner = Base::Owner::UNKNOWN;
				}
			}
		}
		if (info->_owner == Base::Owner::ENEMY)
		{
			//if we see the base and there is no resource depot, set owner to none
			//this is needed for cancelled buildings
			if (Map::IsVisible(base)
				&& BWAPI::Broodwar->getUnitsInRectangle((BWAPI::Position)base->Location(), 
					(BWAPI::Position)base->Location()+BWAPI::Position(128,96),
					BWAPI::Filter::IsResourceDepot).empty())
					info->_owner = Base::Owner::NONE;				
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
		if (unitID == (*it)->_id)
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

void ScoutModule::MergeArmies()
{
	bool allGood = false;
	while (!allGood)
	{
		allGood = true;

		for (auto& x : _armies)
		{
			if (!allGood)
				break;
			for (auto it = _armies.begin(); it != _armies.end(); it++)
			{
				if (x == *it)
					continue;

				if (x->BoundingBox()._center.getDistance((*it)->BoundingBox()._center) < Config::Units::EnemyArmyRange() * TILE_SIZE)
				{
					x->Join((*it).get());
					_armies.erase(it);
					allGood = false;
					break;
				}
			}
		}
	}
}

void ScoutModule::CreateDefendTasks()
{
	for (auto& eArmy : _armies)
	{
		//check distance to our buildings, skip armies that are no threat to our bases
		if(eArmy->IsThreat())
			ArmyModule::Instance()->AddDefendTask(eArmy.get());
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
	ResetBaseInfo();
	MergeArmies();
	CreateDefendTasks();

	for (auto& army : _armies)
	{
		if(!army->Units().empty())
			army->OnFrame();
	}
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
		if (unit->getDistance(BWEB::Map::getNaturalPosition()) > 1000 || unit->getType().isResourceDepot()) //this is a check for cannon rushing
		{
			_enemyStart = Map::ClosestStart(unit->getTilePosition());
			_ASSERT(!_enemyStart->Bases().empty());
			auto station = BWEB::Stations::getClosestNaturalStation(_enemyStart->Bases().front().Location());
			_ASSERT(station && station->getBWEMBase());
			_enemyNatural = station->getBWEMBase()->GetArea();

			//set base as belonging to enemy
			if (_enemyStart && !_enemyStart->Bases().empty())
			{
				((BaseInfo*)_enemyStart->Bases().front().Ptr())->_owner = Base::Owner::ENEMY;
			}
		}
	}

	if (unit->getType().isBuilding()) //reset build items, so they don't try to build on enemy buildings
	{
		ProductionModule::Instance()->TileOccupied(unit);

		//set base ownership to enemy
		if (unit->getType().isResourceDepot())
		{
			((BaseInfo*)Map::GetStation(unit->getTilePosition())->getBWEMBase()->Ptr())->_owner = Base::Owner::ENEMY;
		}
	}

	EnemyUnit* newUnit = nullptr;
	auto it_type = _enemies.find(unit->getType()); //find type

	if (it_type != _enemies.end())
	{
		for (auto& enemy : it_type->second)
		{
			if (enemy->_id == unit->getID()) //find this unit by ID
			{
				enemy->_lastPos = BWAPI::TilePosition(unit->getPosition());
				enemy->_hidden = false;
				return;
			}
		}
		//if not in the list, create new entry
		newUnit = it_type->second.emplace_back(std::make_unique<EnemyUnit>(unit)).get();

	}
	//if first enemy of this type add new vector to list
	else
	{
		auto new_it = _enemies.insert({ unit->getType(), EnemyList{} });
		newUnit = new_it.first->second.emplace_back(std::make_unique<EnemyUnit>(unit)).get();
	}

	AssignToArmy(newUnit);
	CheckEnemyEvolution(unit);
}

void ScoutModule::EnemyHidden(BWAPI::Unit unit)
{
	for(auto& type : _enemies)
	{
		for (auto& enemy : type.second)
		{
			if (enemy->_id == unit->getID()) //find this unit by ID
			{
				enemy->_hidden = true;
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

	//enemy base destroyed
	if (unit->getType().isResourceDepot())
	{
		((BaseInfo*)Map::GetStation(unit->getTilePosition())->getBWEMBase()->Ptr())->_owner = Base::Owner::NONE;
	}

	auto it_type = _enemies.find(unit->getType()); //find type

	if (it_type != _enemies.end())
	{
		for (auto it = it_type->second.begin(); it != it_type->second.end(); it++)
		{
			if ((*it)->_id == unit->getID()) //find this unit by ID
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

bool ScoutModule::ShouldWorkerScoutRush()
{
	if (_enemyRace == BWAPI::Races::Terran || _enemyRace == BWAPI::Races::Zerg)
		return false;

	if (BWAPI::Broodwar->getFrameCount() < 1200) //TODO configurable
		return false;

	if(WorkersModule::Instance()->ExpansionCount() > 1)
		return false;

	return true;
}

int ScoutModule::GetCountOf(BWAPI::UnitType type)
{
	auto it = _enemies.find(type);

	if (it == _enemies.end())
		return 0;
	return it->second.size();
}

BWAPI::Race ScoutModule::GetEnemyRace()
{
	if (_enemyRace != BWAPI::Races::Unknown)
		return _enemyRace;

	for (auto p : BWAPI::Broodwar->getPlayers())
	{
		if (BWAPI::Broodwar->self()->isEnemy(p))
		{
			if (p->getRace() != BWAPI::Races::Random
				&& p->getRace() != BWAPI::Races::Unknown)
			{
				_enemyRace = p->getRace();
				return _enemyRace;
			}
		}
	}

	if (!_enemies.empty())
	{
		auto it = _enemies.begin();
		_enemyRace = it->first.getRace();
	}
	
	return _enemyRace;
}

void ScoutModule::AssignToArmy(EnemyUnit * enemy)
{
	if (!enemy)
		return;

	_ASSERT(enemy->_lastPos != BWAPI::TilePositions::Unknown);

	if (enemy->_type.isWorker() || enemy->_type.isBuilding()) //buildings that are in our main or natural or close to nat choke are assigned to enemy army to trigger worker defence
	{
		auto area = BWEM::Map::Instance().GetNearestArea(enemy->_lastPos);
		if (!area || (area != BWEB::Map::getMainArea() 
			&& area != BWEB::Map::getNaturalArea() 
			&& (BWAPI::Position(enemy->_lastPos)).getDistance((BWAPI::Position)BWEB::Map::getNaturalChoke()->Center()) > 300)) //TODO configurable
			return;
	}

	for (auto& army : _armies)
	{
		if (BWAPI::TilePosition(army->BoundingBox()._center).getDistance(enemy->_lastPos) <= Config::Units::EnemyArmyRange())
		{
			army->AddEnemy(enemy);
			return;
		}
	}
	EnemyArmy* newArmy = _armies.emplace_back(std::make_unique<EnemyArmy>()).get();
	_ASSERT(newArmy);
	newArmy->AddEnemy(enemy);
}
