#include "ZombieManager.h"
#include "Zombie.h"

using namespace std;

ZombieManager::ZombieManager()
{
	zombies.reserve(50);

	zombies.push_back(make_shared<Zombie>(0));
	zombies.push_back(make_shared<Zombie>(1));
}

shared_ptr<Zombie> ZombieManager::GetZombie(const Vector3D& loc, const Rotator& rot)
{
	for (int i=0; i<zombies.size(); i++)
	{
		if (!zombies[i]->GetIsActive())
		{
			zombies[i]->SetLocation(loc);
			zombies[i]->SetRotation(rot);
			zombies[i]->Activate();
			return zombies[i];
		}
	}
	return nullptr;
	// create new zombie
}
