#pragma once
#include "ZombieState.h"
#include "../../Enums/ZombieStateEnum.h"
#include <memory>

class Zombie;

class AttackState : public ZombieState
{
public:

	AttackState() = default;

	virtual ~AttackState() = default;

	virtual void ChangeState(std::shared_ptr<Zombie> zombie) override;

	virtual void Update(std::shared_ptr<Zombie> zombie) override;

	virtual EZombieState GetStateEnum() override;

	static AttackState* GetInstance()
	{
		static AttackState attack;
		return &attack;
	}
};