#include "EnemyStrategy.h"

using namespace KasoBot;

EnemyStrategy::EnemyStrategy(std::string& name, int id)
	:_name(name), _id(id)
{
}

EnemyStrategy::~EnemyStrategy()
{
}

void EnemyStrategy::AddItem(BWAPI::UnitType type, int value, int limit, bool include)
{
	_items.emplace_back(std::make_unique<Enemy::StratItem>(type, value, limit, include));
}
