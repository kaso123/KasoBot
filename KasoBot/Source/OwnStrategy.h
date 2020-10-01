#pragma once
#include <BWAPI.h>

namespace KasoBot {

	class Opener;

	namespace Production {
		struct TechMacro;
	}

	typedef std::pair<BWAPI::UnitType, int> UnitItem;

	class OwnStrategy
	{
	private:
		std::string _name;
		std::vector<UnitItem> _units;
		std::vector<Production::TechMacro> _tech;
		std::string _opener;

	public:
		OwnStrategy(std::string& name, std::string& opener);
		~OwnStrategy();

		void AddUnit(BWAPI::UnitType type, int count);
		
		void AddTech(Production::TechMacro macro);

		//getters and setters

		const std::string& GetOpener() const { return _opener; }
		const std::string& GetName() const { return _name; }
	};

}
