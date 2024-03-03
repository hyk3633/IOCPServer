#pragma once
#include "ZombieState.h"
#include "../Zombie.h"

class PatrolState : public ZombieState
{
public:

	PatrolState() = default;

	virtual ~PatrolState() = default;

	virtual void ChangeState(Zombie* zombie) override;

	virtual void Update(Zombie* zombie) override;

	virtual EZombieState GetStateEnum() override;

	static PatrolState* GetInstance()
	{
		static PatrolState patrol;
		return &patrol;
	}
};