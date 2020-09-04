#include "ProductionItem.h"
#include "MapModule.h"
#include "ProductionModule.h"

using namespace KasoBot;

ProductionItem::ProductionItem(BWAPI::UnitType type)
	:_state(Production::State::WAITING), _type(type), _buildLocation(BWAPI::TilePositions::Invalid)
{
}

ProductionItem::ProductionItem(BWAPI::UnitType type, BWAPI::TilePosition pos)
	: _state(Production::State::WAITING), _type(type), _buildLocation(pos)
{
}

ProductionItem::~ProductionItem()
{
}

void ProductionItem::Assigned()
{
	_ASSERT(_state == Production::State::WAITING);
	_state = Production::State::ASSIGNED;
	ProductionModule::Instance()->ReserveResources(_type);
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
