#pragma once
#include "ZombieState.h"
#include "../../Enums/ZombieStateEnum.h"
#include <memory>

class Zombie;

class BlockedState : public ZombieState
{
public:

	BlockedState() = default;

	virtual ~BlockedState() = default;

	virtual void ChangeState(std::shared_ptr<Zombie> zombie) override;

	virtual void Update(std::shared_ptr<Zombie> zombie) override;

	virtual EZombieState GetStateEnum() override;

	static BlockedState* GetInstance()
	{
		static BlockedState blocked;
		return &blocked;
	}
};