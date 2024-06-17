#pragma once
#include "../Structs/Vector3D.h"
#include "../Structs/Rotator.h"
#include <vector>
#include <memory>
#include <unordered_map>

class Zombie;
class JsonComponent;

class ZombieManager
{
public:

	ZombieManager();
	~ZombieManager() = default;

	void InitZombies(std::unordered_map<int, std::shared_ptr<Zombie>>& zombieMap);

	std::shared_ptr<Zombie> GetNewZombie();

private:

	std::vector<std::shared_ptr<Zombie>> zombies;

	unique_ptr<JsonComponent> jsonComponent;

	std::vector<pair<Vector3D, Rotator>> placedZombies;

};