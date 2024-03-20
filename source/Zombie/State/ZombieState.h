#pragma once
#include "../../Enums/ZombieStateEnum.h"
#include <memory>

using namespace std;

class Zombie;

class ZombieState
{
public:

	ZombieState() = default;

	virtual ~ZombieState() = default;

	virtual void ChangeState(std::shared_ptr<Zombie> zombie) = 0;

	virtual void Update(std::shared_ptr<Zombie> zombie) = 0;

	virtual EZombieState GetStateEnum() = 0;

};