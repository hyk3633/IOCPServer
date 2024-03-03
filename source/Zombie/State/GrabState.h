#pragma once
#include "ZombieState.h"
#include "../Zombie.h"

class GrabState : public ZombieState
{
public:

	GrabState() = default;

	virtual ~GrabState() = default;

	virtual void ChangeState(Zombie* zombie) override;

	virtual void Update(Zombie* zombie) override;

	virtual EZombieState GetStateEnum() override;

	static GrabState* GetInstance()
	{
		static GrabState grab;
		return &grab;
	}
};