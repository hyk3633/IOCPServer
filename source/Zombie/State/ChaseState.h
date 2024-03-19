#pragma once
#include "ZombieState.h"

class Zombie;

class ChaseState : public ZombieState
{
public:

	ChaseState() = default;

	virtual ~ChaseState() = default;

	virtual void ChangeState(Zombie* zombie) override;

	virtual void Update(Zombie* zombie) override;

	virtual EZombieState GetStateEnum() override;

	static ChaseState* GetInstance()
	{
		static ChaseState chase;
		return &chase;
	}
};