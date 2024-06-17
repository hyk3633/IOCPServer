#include "ZombieManager.h"
#include "Zombie.h"
#include "../JsonComponent.h"

using namespace std;

ZombieManager::ZombieManager()
{
	jsonComponent = make_unique<JsonComponent>();

	jsonComponent->GetPlacedZombieInfo(placedZombies);

	zombies.reserve(50);

	for (int i=0; i< placedZombies.size(); i++)
	{
		zombies.push_back(make_shared<Zombie>(i));
		zombies[i]->SetLocation(placedZombies[i].first);
		zombies[i]->SetRotation(placedZombies[i].second);
	}
}

void ZombieManager::InitZombies(std::unordered_map<int, std::shared_ptr<Zombie>>& zombieMap)
{
	for (int i = 0; i < zombies.size(); i++)
	{
		zombieMap[i] = zombies[i];
	}
}

std::shared_ptr<Zombie> ZombieManager::GetNewZombie()
{
	for (int i = 0; i < zombies.size(); i++)
	{
		zombies.push_back(make_shared<Zombie>(i));
		zombies[i]->SetLocation(placedZombies[i].first);
		zombies[i]->SetRotation(placedZombies[i].second);
		return zombies[i];
	}
}
