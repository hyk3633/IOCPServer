#pragma once
#include "ZombieState.h"
#include "../../Enums/ZombieStateEnum.h"
#include <memory>

class Zombie;

class IdleState : public ZombieState
{
public:

	IdleState() = default;

	virtual ~IdleState() = default;

	virtual void ChangeState(std::shared_ptr<Zombie> zombie) override;

	virtual void Update(std::shared_ptr<Zombie> zombie) override;

	virtual EZombieState GetStateEnum() override;

	static IdleState* GetInstance()
	{
		static IdleState idle;
		return &idle;
	}
};