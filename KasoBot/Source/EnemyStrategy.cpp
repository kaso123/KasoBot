#include "EnemyStrategy.h"
#include "ScoutModule.h"

using namespace KasoBot;

int EnemyStrategy::Evaluate(Enemy::StratItem & item)
{
	int count = ScoutModule::Instance()->GetCountOf(item._type);

	if (item._include) //this strat contains this type
	{
		if (item._limit <= count)
			return item._value;
	}
	else //this strategy should not contain this type
	{
		if (item._limit <= count)
			return -(item._value);
	}

	return 0;
}

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

void EnemyStrategy::AddCounter(std::string & name)
{
	_counters.emplace_back(name);
}

int EnemyStrategy::Score()
{
	int result = 0;

	for (auto& item : _items)
	{
		result += Evaluate(*item);
	}

	return result;
}
