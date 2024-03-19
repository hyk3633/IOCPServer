#pragma once
#include <vector>
#include "../Structs/Pos.h"
#include "../Structs/Vector3D.h"

using namespace std;

class Zombie;

class PathManager
{
public:

	PathManager();

	~PathManager();

	void ProcessMovement();

protected:

	void InitializePathStatus();

	void FollowPath();

	bool WhetherRecalculPath();

	void ClearPathStatus();

public:

	inline void SetZombie(Zombie* newZombie) { zombie = newZombie; }

	inline vector<Pos>& GetPathToTarget() { return pathToTarget; }

private:

	Zombie* zombie;

	vector<Pos> pathToTarget;

	vector<Pos> pathIndexArr;

	int pathIdx;

	Vector3D nextPoint;

	Vector3D nextDirection;

};