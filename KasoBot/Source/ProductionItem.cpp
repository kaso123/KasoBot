#include "ProductionItem.h"

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
