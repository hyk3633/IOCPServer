#pragma once
#include "ZombieState.h"
#include "../../Enums/ZombieStateEnum.h"
#include <memory>

class Zombie;

class WaitState : public ZombieState
{
public:

	WaitState() = default;

	virtual ~WaitState() = default;

	virtual void ChangeState(std::shared_ptr<Zombie> zombie) override;

	virtual void Update(std::shared_ptr<Zombie> zombie) override;

	virtual EZombieState GetStateEnum() override;

	static WaitState* GetInstance()
	{
		static WaitState wait;
		return &wait;
	}
};