#pragma once
#include "ZombieState.h"
#include "../../Enums/ZombieStateEnum.h"
#include <memory>

class Zombie;

class ChaseState : public ZombieState
{
public:

	ChaseState() = default;

	virtual ~ChaseState() = default;

	virtual void ChangeState(std::shared_ptr<Zombie> zombie) override;

	virtual void Update(std::shared_ptr<Zombie> zombie) override;

	virtual EZombieState GetStateEnum() override;

	static ChaseState* GetInstance()
	{
		static ChaseState chase;
		return &chase;
	}
};