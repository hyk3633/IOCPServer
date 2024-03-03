#pragma once
#include "ZombieState.h"
#include "../Zombie.h"

class BlockedState : public ZombieState
{
public:

	BlockedState() = default;

	virtual ~BlockedState() = default;

	virtual void ChangeState(Zombie* zombie) override;

	virtual void Update(Zombie* zombie) override;

	virtual EZombieState GetStateEnum() override;

	static BlockedState* GetInstance()
	{
		static BlockedState blocked;
		return &blocked;
	}
};