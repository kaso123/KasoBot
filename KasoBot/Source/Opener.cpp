#include "Opener.h"
#include "Config.h"
#include "ProductionModule.h"

using namespace KasoBot;

Opener::Opener(nlohmann::json & j)
{
	_ASSERT(j.is_array());

	for (auto item : j)
	{
		_ASSERT(item.is_string());
		std::string type = item.get<std::string>();

		if (type.find(" $") != std::string::npos) //multiplication in opener
		{
			int count = stoi(type.substr(type.find("$") + 1, type.length() - type.find("$")));

			type = type.substr(0, type.find(" $"));
			for (int i = 0; i < count; i++)
			{
				_queue.emplace_back(Config::Utils::TypeFromString(type));
			}
			continue;
		}

		//single instance of unit
		_queue.emplace_back(Config::Utils::TypeFromString(type));
	}
}

Opener::~Opener()
{
}

bool Opener::Pop()
{
	_queue.pop_front();
	
	return _queue.size() <= 0;
}

bool Opener::ResetProgress()
{
	std::deque<BWAPI::UnitType> newQueue;

	//check how much of opener is already done
	std::map<BWAPI::UnitType, int> previous;
	
	//add starting units to map
	previous.emplace(BWAPI::UnitTypes::Terran_Command_Center, 1);
	previous.emplace(BWAPI::UnitTypes::Terran_SCV, 4);
	
	for (auto& type : _queue)
	{
		auto typeIt = previous.find(type);
		if (typeIt != previous.end()) //not first
			typeIt->second++;
		else typeIt = previous.emplace(type, 1).first;

		if (ProductionModule::Instance()->GetCountOf(typeIt->first) >= typeIt->second)
			continue;

		newQueue.emplace_back(type);
	}

	_queue.clear();
	_queue = newQueue;

	return _queue.empty();
}
