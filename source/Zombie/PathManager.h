#pragma once
#include "../Structs/Pos.h"
#include "../Structs/Vector3D.h"
#include <vector>
#include <memory>

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

	inline void SetZombie(std::shared_ptr<Zombie> newZombie) { zombie = newZombie; }

	inline std::vector<Pos>& GetPathToTarget() { return pathToTarget; }

private:

	std::shared_ptr<Zombie> zombie;

	std::vector<Pos> pathToTarget;

	std::vector<Pos> pathIndexArr;

	int pathIdx;

	Vector3D nextPoint;

	Vector3D nextDirection;

};