#pragma once
#include "ZombieState.h"
#include "../Zombie.h"

class WaitState : public ZombieState
{
public:

	WaitState() = default;

	virtual ~WaitState() = default;

	virtual void ChangeState(Zombie* zombie) override;

	virtual void Update(Zombie* zombie) override;

	virtual EZombieState GetStateEnum() override;

	static WaitState* GetInstance()
	{
		static WaitState wait;
		return &wait;
	}
};