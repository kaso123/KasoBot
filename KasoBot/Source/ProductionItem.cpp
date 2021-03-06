#include "ProductionItem.h"
#include "MapModule.h"
#include "ProductionModule.h"
#include "WorkersModule.h"
#include "ArmyModule.h"
#include "Config.h"
#include "Log.h"

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
	Log::Instance()->Assert(_state == Production::State::WAITING || _state == Production::State::UNFINISHED,"Assigning item in wrong state!");

	_state = Production::State::ASSIGNED;

	Log::Instance()->Assert(_buildLocation.isValid(), "Invalid location when assigned prod item!");

	//check if this is natural base and move default task if needed
	if (_buildLocation.isValid() && _buildLocation == BWEB::Map::getNaturalTile())
		ArmyModule::Instance()->ResetDefaultTask();
}

void ProductionItem::BuildStarted()
{
	Log::Instance()->Assert(_state == Production::State::ASSIGNED,"Started build in wrong state!");
	Log::Instance()->Assert(_buildLocation.isValid(),"Invalid location whe starting build!");

	if (!_unfinished)
	{
		BWEB::Map::KasoBot::UnreserveTiles(_buildLocation, _type);
		ProductionModule::Instance()->FreeResources(_type);
	}
	
	_state = Production::State::BUILDING;
}

void ProductionItem::Restart()
{
	Log::Instance()->Assert(_state == Production::State::BUILDING,"Restarted item is not in building state!");
	Log::Instance()->Assert(_buildLocation.isValid(),"Invalid location for production item!");

	ProductionModule::Instance()->ReserveResources(_type);
	BWEB::Map::KasoBot::ReserveTiles(_buildLocation, _type);
	_state = Production::State::ASSIGNED;
}

void ProductionItem::Finish()
{
	Log::Instance()->Assert(_state == Production::State::BUILDING,"Wrong state in finished production item!");

	_state = Production::State::DONE;
}

void ProductionItem::WorkerDied()
{
	//timeout only outside main area to better counter worker rushes
	_timeout = BWAPI::Broodwar->getFrameCount() + (BWEM::Map::Instance().GetArea(_buildLocation) == BWEB::Map::getMainArea() ? 0 : Config::Production::BuildTimeout());

	Log::Instance()->Assert(_state == Production::State::ASSIGNED || _state == Production::State::BUILDING,"Wrong state when assigned worker died!");

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
	//timeout only outside main area to better counter worker rushes
	_timeout = BWAPI::Broodwar->getFrameCount() + (BWEM::Map::Instance().GetArea(_buildLocation) == BWEB::Map::getMainArea() ? 0 : Config::Production::BuildTimeout());

	if (_state == Production::State::BUILDING)
	{
		_state = Production::State::WAITING;
		BWEB::Map::KasoBot::ReserveTiles(_buildLocation, _type);
		ProductionModule::Instance()->ReserveResources(_type);
		WorkersModule::Instance()->BuildFailed(this);
	}
	else if (_state == Production::State::UNFINISHED)
	{
		_unfinished = false;
		_state = Production::State::WAITING;
		BWEB::Map::KasoBot::ReserveTiles(_buildLocation, _type);
		ProductionModule::Instance()->ReserveResources(_type);
	}
	else if (_state == Production::State::ASSIGNED && !_unfinished)
	{
		Log::Instance()->Assert(false, "Building destroyed, but was in wrong state!");
		_state = Production::State::WAITING;
		WorkersModule::Instance()->BuildFailed(this);
	}
	else if (_state == Production::State::ASSIGNED && _unfinished)
	{
		_unfinished = false;
		_state = Production::State::WAITING;
		BWEB::Map::KasoBot::ReserveTiles(_buildLocation, _type);
		ProductionModule::Instance()->ReserveResources(_type);
		WorkersModule::Instance()->BuildFailed(this);	
	}
	else Log::Instance()->Assert(false, "Wrong state on destroyed incomplete building!"); //this is only called on incomplete buildings, so there can't be any other state

	if (_buildLocation.isValid() && _buildLocation == BWEB::Map::getNaturalTile())
		ArmyModule::Instance()->ResetDefaultTask();
}
