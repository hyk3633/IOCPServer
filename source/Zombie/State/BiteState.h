#pragma once
#include "ZombieState.h"
#include "../Zombie.h"

class BiteState : public ZombieState
{
public:

	BiteState() = default;

	virtual ~BiteState() = default;

	virtual void ChangeState(Zombie* zombie) override;

	virtual void Update(Zombie* zombie) override;

	virtual EZombieState GetStateEnum() override;

	static BiteState* GetInstance()
	{
		static BiteState bite;
		return &bite;
	}
};