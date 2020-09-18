#include "Army.h"
#include "Unit.h"
#include "Config.h"

using namespace KasoBot;

Army::Army()
{
}

Army::~Army()
{
}

bool Army::AddSoldier(KasoBot::Unit* unit)
{
	if (GetSupply() + unit->GetPointer()->getType().supplyRequired() > Config::Strategy::MaxArmySupply() * 2)
		return false;

	_soldiers.emplace_back(unit);
	return true;
}

bool Army::SoldierKilled(KasoBot::Unit* unit)
{
	for (auto it = _soldiers.begin(); it != _soldiers.end(); it++)
	{
		if (*it == unit)
		{
			_soldiers.erase(it);
			return true;
		}
	}

	return false;
}

int Army::GetSupply()
{
	int supply = 0;
	for (auto& unit : _soldiers)
	{
		supply += unit->GetPointer()->getType().supplyRequired();
	}

	return supply;
}

void Army::ClearTiles(BWAPI::TilePosition pos, BWAPI::UnitType type)
{
	for (auto& soldier : _soldiers)
	{
		int x = soldier->GetPointer()->getTilePosition().x;
		int y = soldier->GetPointer()->getTilePosition().y;

		if ((pos.x) - 1 <= x && x < (pos.x + type.tileWidth())
			&& (pos.y) - 1 <= y && y < (pos.y + type.tileHeight()))
			soldier->ClearTile();
	}
}
