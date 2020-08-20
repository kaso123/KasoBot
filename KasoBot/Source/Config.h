#pragma once
#include <BWAPI.h>

namespace KasoBot {

	class ConfigModule
	{
	private:
		ConfigModule();
		~ConfigModule();
		static ConfigModule* _instance;

		int _maxWorkersTotal = 80;
		int _maxWorkersPerMineral = 2;
		int _maxWorkersPerGas = 3;

	public:
		static ConfigModule* Instance();

		void Init();

		int MaxWorkersTotal() { return _maxWorkersTotal; };
		int MaxWorkersPerMineral() { return _maxWorkersPerMineral; };
		int MaxWorkersPerGas() { return _maxWorkersPerGas; };
	};

	namespace Config {
		namespace Workers {

			//max allowed worker number globally
			int MaxGlobal(); 

			//max workers allowed per one mineral
			int MaxPerMineral(); 
			
			//max workers allowed per one refinery
			int MaxPerGas();
		}
	}
}


