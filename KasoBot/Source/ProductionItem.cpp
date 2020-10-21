#include "ProductionItem.h"
#include "MapModule.h"
#include "ProductionModule.h"
#include "WorkersModule.h"
#include "Config.h"

using namespace KasoBot;

ProductionItem::ProductionItem(BWAPI::UnitType type)
	:_state(Production::State::WAITING), _type(type), _buildLocation(BWAPI::TilePositions::Invalid), _unfinished(false)
	, _timeout(0)
{
	ProductionModule::Instance()->ReserveResources(_type);
}

ProductionItem::ProductionItem(BWAPI::UnitType type, BWAPI::TilePosition pos)
	: _state(Production::State::WAITING), _type(type), _buildLocation(pos), _unfinished(false)
	, _timeout(0)
{
	ProductionModule::Instance()->ReserveResources(_type);
	BWEB::Map::KasoBot::ReserveTiles(_buildLocation, _type);
}

ProductionItem::~ProductionItem()
{
}

void ProductionItem::Assigned()
{
	_ASSERT(_state == Production::State::WAITING || _state == Production::State::UNFINISHED);
	_state = Production::State::ASSIGNED;
}

void ProductionItem::BuildStarted()
{
	_ASSERT(_state == Production::State::ASSIGNED);
	_ASSERT(_buildLocation.isValid());

	if (!_unfinished)
	{
		BWEB::Map::KasoBot::UnreserveTiles(_buildLocation, _type);
		ProductionModule::Instance()->FreeResources(_type);
	}
	
	_state = Production::State::BUILDING;
}

void ProductionItem::Restart()
{
	_ASSERT(_state == Production::State::BUILDING);
	_ASSERT(_buildLocation.isValid());

	ProductionModule::Instance()->ReserveResources(_type);
	BWEB::Map::KasoBot::ReserveTiles(_buildLocation, _type);
	_state = Production::State::ASSIGNED;
}

void ProductionItem::Finish()
{
	_ASSERT(_state == Production::State::BUILDING);

	_state = Production::State::DONE;
}

void ProductionItem::WorkerDied()
{
	_timeout = BWAPI::Broodwar->getFrameCount() + Config::Production::BuildTimeout();

	_ASSERT(_state == Production::State::ASSIGNED || _state == Production::State::BUILDING);

	if (_state == Production::State::ASSIGNED)
	{
		//when only assigned -> go to waiting
		if(_unfinished)
			_state = Production::State::UNFINISHED;
		else _state = Production::State::WAITING;
		return;
	}

	if (_state == Production::State::BUILDING)
	{
		//when already built
		_state = Production::State::UNFINISHED;
		_unfinished = true;
		return;
	}
}

void ProductionItem::BuildingDestroyed()
{
	_timeout = BWAPI::Broodwar->getFrameCount() + Config::Production::BuildTimeout();
	
	if (_state == Production::State::BUILDING)
	{
		_state = Production::State::WAITING;
		BWEB::Map::KasoBot::ReserveTiles(_buildLocation, _type);
		ProductionModule::Instance()->ReserveResources(_type);
		WorkersModule::Instance()->BuildFailed(this);
		return;
	}

	if (_state == Production::State::UNFINISHED)
	{
		_unfinished = false;
		_state = Production::State::WAITING;
		BWEB::Map::KasoBot::ReserveTiles(_buildLocation, _type);
		ProductionModule::Instance()->ReserveResources(_type);
		return;
	}
	if (_state == Production::State::ASSIGNED && !_unfinished)
	{
		_state = Production::State::WAITING;
		WorkersModule::Instance()->BuildFailed(this);
		return;
	}
	if (_state == Production::State::ASSIGNED && _unfinished)
	{
		_unfinished = false;
		_state = Production::State::WAITING;
		BWEB::Map::KasoBot::ReserveTiles(_buildLocation, _type);
		ProductionModule::Instance()->ReserveResources(_type);
		WorkersModule::Instance()->BuildFailed(this);
		return;
	}

	_ASSERT(false); //this is only called on incomplete buildings, so there can't be any other state
}
