#pragma once
#include "ZombieState.h"
#include "../Zombie.h"

class AttackState : public ZombieState
{
public:

	AttackState() = default;

	virtual ~AttackState() = default;

	virtual void ChangeState(Zombie* zombie) override;

	virtual void Update(Zombie* zombie) override;

	virtual EZombieState GetStateEnum() override;

	static AttackState* GetInstance()
	{
		static AttackState attack;
		return &attack;
	}
};