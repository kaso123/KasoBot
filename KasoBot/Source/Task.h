#pragma once
#include <BWAPI.h>

namespace BWEM {
	class Area;
}

namespace KasoBot {

	class EnemyArmy;
	class Army;

	namespace Tasks {
		enum Type { //the order is important for sorting tasks
			DEFEND,
			ATTACK,
			HARASS,
			HUNT,
			FINISH,
			SCOUT,
			SUPPORT,
			HOLD
		};
	}

	class Task
	{
	protected:
		Task(Tasks::Type type);
		Tasks::Type _type;
		bool _inProgress;
		bool _finished;

	public:
		virtual ~Task();

		//@return true if army can be selected for this task
		virtual bool IsArmySuitable(Army& army) { return false; }

		//check if the goal of this task was accomplished
		virtual bool IsFinished() { return false; }

		//getters and setters

		virtual void Start() { _inProgress = true; }
		void Stop() { _inProgress = false; }
		Tasks::Type Type() const { return _type; }
		virtual BWAPI::Position Position() const { return BWAPI::Positions::Invalid; }
		virtual const BWEM::Area* Area() const { return nullptr; }
		virtual EnemyArmy* EnemyArmy() const { return nullptr; }
		virtual Army* FriendlyArmy() const { return nullptr; }
		virtual BWAPI::TilePosition Next() { return BWAPI::TilePositions::Invalid; }

		virtual bool IsCheckpointDone() const { return true; }
		virtual void SetCheckpointDone() {};

		bool InProgress() { return _inProgress; }
	};

	class AttackAreaTask : public Task {
	private:
		const BWEM::Area* _area;
	public:
		AttackAreaTask(const BWEM::Area* area);
		~AttackAreaTask() {};

		bool IsArmySuitable(Army& army) override;
		bool IsFinished() override;


		const BWEM::Area* Area() const override { return _area; }
		
	};

	class HoldPositionTask : public Task {
	private:
		BWAPI::Position _pos;
	public:
		HoldPositionTask(BWAPI::Position pos);
		~HoldPositionTask() {};

		bool IsArmySuitable(Army& army) override { return true; }

		//getteres and setters

		BWAPI::Position Position() const override { return _pos; }
	};

	class DefendArmyTask  : public Task {
	private:
		KasoBot::EnemyArmy* _army;
	public:
		DefendArmyTask(KasoBot::EnemyArmy* army);
		~DefendArmyTask() {};

		bool IsArmySuitable(Army& army) override;

		bool IsFinished() override;

		//getters and setters
		
		KasoBot::EnemyArmy* EnemyArmy() const override { return _army; }
	};

	class ScoutAreaTask : public Task {
	private:
		const BWEM::Area* _area;
	public:
		ScoutAreaTask(const BWEM::Area* area);
		~ScoutAreaTask() {};

		bool IsArmySuitable(Army& army) override;

		bool IsFinished() override;

		//getters and setters

		const BWEM::Area* Area() const override { return _area; }
	};

	class FinishEnemyTask : public Task {
	private:
		BWAPI::TilePosition _nextPos;
	public:
		FinishEnemyTask();
		~FinishEnemyTask() {};

		bool IsArmySuitable(Army& army) override;
		bool IsFinished() override { return false; };

		BWAPI::TilePosition Next() override;
	};

	class SupportArmyTask : public Task {
	private:
		Army* _army;
	public:
		SupportArmyTask(KasoBot::Army* army);
		~SupportArmyTask() {};

		bool IsArmySuitable(KasoBot::Army& army) override;
		bool IsFinished() override;

		Army* FriendlyArmy() const override { return _army; }
	};

	class HarassAreaTask : public Task {
	private:
		const BWEM::Area* _area;
		bool _cp;
	public:
		HarassAreaTask(const BWEM::Area* area);
		~HarassAreaTask() {};

		void Start() override;

		void SetCheckpointDone() override { _cp = true; }
		bool IsArmySuitable(Army& army) override;
		bool IsFinished() override;

		bool IsCheckpointDone() const override { return _cp; }
		const BWEM::Area* Area() const override { return _area; }
	};
}
