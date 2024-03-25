#pragma once
#include "../Structs/Vector3D.h"
#include "../Structs/Rotator.h"
#include <vector>
#include <memory>

class Zombie;

class ZombieManager
{
public:

	ZombieManager();
	~ZombieManager() = default;

	std::shared_ptr<Zombie> GetZombie(const Vector3D& loc, const Rotator& rot);

private:

	std::vector<std::shared_ptr<Zombie>> zombies;

};