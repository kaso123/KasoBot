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
		int _maxWorkersPerMineral = 3;
		int _maxWorkersPerGas = 3;
		int _saturatedMineral = 2;
		int _saturatedGas = 3;

	public:
		static ConfigModule* Instance();

		void Init();

		int MaxWorkersTotal() { return _maxWorkersTotal; }
		int MaxWorkersPerMineral() { return _maxWorkersPerMineral; }
		int MaxWorkersPerGas() { return _maxWorkersPerGas; }
		int SaturatedMineral() { return _saturatedMineral; }
		int SaturatedGas() { return _saturatedGas; }
	};

	namespace Config {
		namespace Workers {

			//max allowed worker number globally
			int MaxGlobal(); 

			//max workers allowed per one mineral
			int MaxPerMineral(); 
			
			//max workers allowed per one refinery
			int MaxPerGas();

			//ideal number of workers allowed per one mineral
			int SaturationPerMineral();

			//ideal number of workers per one refinery
			int SaturationPerGas();
		}
	}
}


