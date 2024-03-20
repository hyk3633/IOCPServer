#pragma once
#include "ZombieState.h"
#include "../../Enums/ZombieStateEnum.h"
#include <memory>

class Zombie;

class GrabState : public ZombieState
{
public:

	GrabState() = default;

	virtual ~GrabState() = default;

	virtual void ChangeState(std::shared_ptr<Zombie> zombie) override;

	virtual void Update(std::shared_ptr<Zombie> zombie) override;

	virtual EZombieState GetStateEnum() override;

	static GrabState* GetInstance()
	{
		static GrabState grab;
		return &grab;
	}
};