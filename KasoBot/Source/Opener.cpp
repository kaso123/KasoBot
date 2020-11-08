#include "Opener.h"
#include "Config.h"
#include "ProductionModule.h"
#include "Log.h"

using namespace KasoBot;

Opener::Opener(nlohmann::json & j)
{
	Log::Instance()->Assert(j.is_array(),"Wrong json input for opener!");

	for (auto item : j)
	{
		Log::Instance()->Assert(item.is_string(),"Wrong json format in field!");
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

void Opener::Insert(BWAPI::UnitType type)
{
	if (type.isWorker() && (_queue.front() == type || ProductionModule::Instance()->InProgressUnitCount(type) > 0))
		return;
	
	_queue.emplace_front(type);
}
