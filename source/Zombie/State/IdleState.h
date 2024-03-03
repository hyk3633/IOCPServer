#pragma once
#include "ZombieState.h"

class Zombie;

class IdleState : public ZombieState
{
public:

	IdleState() = default;

	virtual ~IdleState() = default;

	virtual void ChangeState(Zombie* zombie) override;

	virtual void Update(Zombie* zombie) override;

	virtual EZombieState GetStateEnum() override;

	static IdleState* GetInstance()
	{
		static IdleState idle;
		return &idle;
	}
};