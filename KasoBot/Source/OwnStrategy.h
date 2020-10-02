#pragma once
#include <BWAPI.h>

namespace KasoBot {

	class Opener;

	namespace Production {
		struct TechMacro;
	}

	struct UnitItem {
		BWAPI::UnitType _type;
		int _value;
		float _proportion;
		UnitItem(BWAPI::UnitType type, int value)
			:_type(type), _value(value), _proportion(-1.0f) {};
	};

	class OwnStrategy
	{
	private:
		std::string _name;
		std::vector<UnitItem> _units;
		std::vector<UnitItem> _production;
		std::vector<Production::TechMacro> _tech;
		std::string _opener;

		//get new tech to do after everything specified in strategy tech path was done
		Production::TechMacro GetMacroAfterTechPathDone() const;

	public:
		OwnStrategy(std::string& name, std::string& opener);
		~OwnStrategy();

		void AddUnit(BWAPI::UnitType type, int count);
		
		void AddTech(Production::TechMacro macro);

		//get all units and calculate desired ratio of production buildings
		void CalculateProduction();

		//@return vector with next army units that should be built in order of priority
		std::vector<BWAPI::UnitType> GetMacroArmyTypes();

		//@return next production building that should be built
		BWAPI::UnitType GetMacroProductionType();

		//@return next upgrade, tech or building that should be built
		Production::TechMacro GetMacroTechType() const;


		//getters and setters

		const std::string& GetOpener() const { return _opener; }
		const std::string& GetName() const { return _name; }
		const std::vector<UnitItem>& GetUnitItems() const { return _units; }
		const std::vector<UnitItem>& GetProductionItems() const { return _production; }
	};

}
