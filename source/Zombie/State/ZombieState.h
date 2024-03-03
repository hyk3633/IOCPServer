#pragma once
#include "../../Pathfinder/PathFinder.h"

using namespace std;

class Zombie;

enum class EZombieState
{
	IDLE,
	PATROL,
	CHASE,
	ATTACK,
	GRAB,
	BITE,
	BLOCKED,
	WAIT
};

class ZombieState
{
public:

	ZombieState() = default;

	virtual ~ZombieState() = default;

	virtual void ChangeState(Zombie* zombie) = 0;

	virtual void Update(Zombie* zombie) = 0;

	virtual EZombieState GetStateEnum() = 0;

protected:

	static Pathfinder* GetPathfinder()
	{
		static Pathfinder pathfinder;
		return &pathfinder;
	}

};