#include "Army.h"
#include "Unit.h"
#include "Config.h"
#include "Task.h"

using namespace KasoBot;

void Army::CalculateCenter()
{
	int minX = INT_MAX;
	int minY = INT_MAX;
	int maxX = INT_MIN;
	int maxY = INT_MIN;

	_ASSERT(!_soldiers.empty());

	for (auto& unit : _soldiers)
	{
		auto pos = unit->GetPointer()->getPosition();
		if (pos.x < minX) minX = pos.x;
		if (pos.x > maxX) maxX = pos.x;
		if (pos.y < minY) minY = pos.y;
		if (pos.y > maxY) maxY = pos.y;
	}

	_box->_topLeft = BWAPI::Position(minX, minY);
	_box->_bottomRight = BWAPI::Position(maxX, maxY);
	_box->_center = BWAPI::Position(minX + (maxX - minX) / 2, minY + (maxY - minY) / 2);
}

Army::Army()
	:_task(nullptr)
{
	_box = std::make_unique<Armies::Box>();
	_box->_topLeft = BWAPI::Position(0, 0);
	_box->_bottomRight = BWAPI::Position(0, 0);
	_box->_center = BWAPI::Position(0, 0);
}

Army::~Army()
{
	if (_task)
		_task->Stop();
}

void Army::OnFrame()
{
	CalculateCenter();
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

void Army::AssignTask(KasoBot::Task * task)
{
	if (_task)
		_task->Stop();

	_task = task;
	_task->Start();
}
