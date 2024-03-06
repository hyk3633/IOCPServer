#pragma once
#include <vector>
#include "../Define.h"

using namespace std;

class Zombie;

class PathManager
{
public:

	PathManager();

	~PathManager();

	void InitializePathStatus();

	void FollowPath();

	bool WhetherRecalculPath();

	void CorrectZombieLocation();

	inline void SetZombie(Zombie* newZombie) { zombie = newZombie; }

	inline vector<Pos>& GetPathToTarget() { return pathToTarget; }

private:

	Zombie* zombie;

	vector<Pos> pathToTarget;

	int pathIdx;

	Vector3D nextPoint;

	Vector3D nextDirection;

	float estimatedTime = 1.5f;

	float elapsedTime;

};