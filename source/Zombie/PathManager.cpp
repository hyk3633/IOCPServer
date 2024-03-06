#include "PathManager.h"
#include "Zombie.h"

PathManager::PathManager()
{

}

PathManager::~PathManager()
{

}

void PathManager::InitializePathStatus()
{
	zombie->SetPath(pathToTarget);
	pathIdx = 0;
	nextPoint = Vector3D(pathToTarget[1].x, pathToTarget[1].y, zombie->GetZombieLocation().Z);
	nextDirection = (nextPoint - zombie->GetZombieLocation()).Normalize();
}

void PathManager::FollowPath()
{
	const float dist = nextPoint.GetDistance(zombie->GetZombieLocation());
	if (dist <= GRID_DIST)
	{
		if (pathIdx < pathToTarget.size())
		{
			nextPoint = Vector3D(pathToTarget[pathIdx].x, pathToTarget[pathIdx].y, zombie->GetZombieLocation().Z);
			nextDirection = (nextPoint - zombie->GetZombieLocation()).Normalize();
			pathIdx++;
		}
	}
	zombie->AddMovement(nextDirection, nextPoint);
}

bool PathManager::WhetherRecalculPath()
{
	if (pathToTarget.size() == 0 || pathIdx >= pathToTarget.size())
	{
		return true;
	}
	else
	{
		// 타겟 위치와 경로의 마지막 위치 간 거리가 100 이상이면 경로 다시 계산
		Vector3D targetLocation = zombie->GetTargetLocation();
		const float destToTarget = targetLocation.GetDistance(Vector3D(pathToTarget.back().x, pathToTarget.back().y, targetLocation.Z));
		if (destToTarget > 100.f)
		{
			return true;
		}
	}
	return false;
}
