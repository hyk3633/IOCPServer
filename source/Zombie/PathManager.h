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

	// 경로를 다시 계산해야 하는지 검사
	bool WhetherRecalculPath();

public:

	inline void SetZombie(std::shared_ptr<Zombie> newZombie) { zombie = newZombie; }

	inline std::vector<Pos>& GetPathToTarget() { return pathToTarget; }

	void ClearPathStatus();

private:

	std::shared_ptr<Zombie> zombie;

	std::vector<Pos> pathToTarget;

	std::vector<Pos> pathIndexArr;

	int pathIdx;

	Vector3D nextPoint;

	Vector3D nextDirection;

	float walkSpeed = 100.f;

};