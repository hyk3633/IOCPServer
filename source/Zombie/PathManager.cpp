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
	else
	{
		zombie->AddMovement(nextDirection);
	}
}

bool PathManager::WhetherRecalculPath()
{
	if (pathToTarget.size() == 0 || pathIdx >= pathToTarget.size())
		return true;
	else
	{
		Vector3D targetLocation = zombie->GetTargetLocation();
		const float destToTarget = targetLocation.GetDistance(Vector3D(pathToTarget.back().x, pathToTarget.back().y, targetLocation.Z));
		if (destToTarget > 100.f)
			return true;
	}
	return false;
}
