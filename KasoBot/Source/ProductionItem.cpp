#include "ProductionItem.h"
#include "MapModule.h"
#include "ProductionModule.h"
#include "WorkersModule.h"

using namespace KasoBot;

ProductionItem::ProductionItem(BWAPI::UnitType type)
	:_state(Production::State::WAITING), _type(type), _buildLocation(BWAPI::TilePositions::Invalid)
{
	ProductionModule::Instance()->ReserveResources(_type);
}

ProductionItem::ProductionItem(BWAPI::UnitType type, BWAPI::TilePosition pos)
	: _state(Production::State::WAITING), _type(type), _buildLocation(pos)
{
	ProductionModule::Instance()->ReserveResources(_type);
}

ProductionItem::~ProductionItem()
{
}

void ProductionItem::Assigned()
{
	_ASSERT(_state == Production::State::WAITING);
	_state = Production::State::ASSIGNED;
}

void ProductionItem::BuildStarted()
{
	_ASSERT(_state == Production::State::ASSIGNED);
	_ASSERT(_buildLocation.isValid());

	BWEB::Map::KasoBot::UnreserveTiles(_buildLocation, _type);
	
	_state = Production::State::BUILDING;
	ProductionModule::Instance()->FreeResources(_type);
}

void ProductionItem::Restart()
{
	_ASSERT(_state == Production::State::BUILDING);
	_ASSERT(_buildLocation.isValid());

	BWEB::Map::KasoBot::UnreserveTiles(_buildLocation, _type);
	_state = Production::State::ASSIGNED;
}

void ProductionItem::Finish()
{
	_ASSERT(_state == Production::State::BUILDING);

	_state = Production::State::DONE;
}

void ProductionItem::WorkerDied()
{
	_ASSERT(_state == Production::State::ASSIGNED || _state == Production::State::BUILDING);

	if (_state == Production::State::ASSIGNED)
	{
		//when only assigned -> remove reserved minerals and go to WAITING
		ProductionModule::Instance()->FreeResources(_type);
		_state = Production::State::WAITING;
		return;
	}

	if (_state == Production::State::BUILDING)
	{
		//when already built
		_state = Production::State::UNFINISHED;
		return;
	}
}

void ProductionItem::BuildingDestroyed()
{
	if (_state == Production::State::BUILDING)
	{
		_state = Production::State::WAITING;
		WorkersModule::Instance()->BuildFailed(this);
		//TODO figure out a worker
		return;
	}

	if (_state == Production::State::UNFINISHED)
	{
		_state = Production::State::WAITING;
		return;
	}

	_ASSERT(false); //this is only called on incomplete buildings, so there can't be any other state
}
