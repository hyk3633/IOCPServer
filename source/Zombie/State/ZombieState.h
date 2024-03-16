#pragma once
using namespace std;

class Zombie;

enum class EZombieState
{
	IDLE,
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

};