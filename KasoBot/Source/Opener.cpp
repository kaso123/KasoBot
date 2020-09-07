#include "Opener.h"
#include "Config.h"

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
