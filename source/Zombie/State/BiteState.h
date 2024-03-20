#pragma once
#include "ZombieState.h"
#include "../../Enums/ZombieStateEnum.h"
#include <memory>

class Zombie;

class BiteState : public ZombieState
{
public:

	BiteState() = default;

	virtual ~BiteState() = default;

	virtual void ChangeState(std::shared_ptr<Zombie> zombie) override;

	virtual void Update(std::shared_ptr<Zombie> zombie) override;

	virtual EZombieState GetStateEnum() override;

	static BiteState* GetInstance()
	{
		static BiteState bite;
		return &bite;
	}
};