#pragma once
#include <BWAPI.h>

namespace KasoBot {
	
	namespace Enemy {
		struct StratItem {
			BWAPI::UnitType _type;
			int _value; //how many points does this strat get if this is true
			int _limit; //count of units of selected types
			bool _include; //true = get points if there is this unit or more than limit, false = get points if there are less units than limit
			
			StratItem(BWAPI::UnitType type, int value, int limit, bool include)
				:_type(type),_value(value),_limit(limit),_include(include){};
		};
	}
	class EnemyStrategy
	{
	private:
		std::string _name;
		int _id;
		std::vector<std::unique_ptr<Enemy::StratItem>> _items;

		//get score for this specific item
		int Evaluate(Enemy::StratItem& item);

	public:
		EnemyStrategy(std::string& name, int id);
		~EnemyStrategy();

		void AddItem(BWAPI::UnitType type, int value, int limit, bool include);


		//get score for this strategy considering known enemy buildings and units
		int Score();

		//getters and setters

		const std::string& GetName() const { return _name; }
		const int GetID() const { return _id; }
	};
}
