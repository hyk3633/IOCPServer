#include "PathManager.h"
#include "Zombie.h"
#include "State/ZombieState.h"
#include "../Pathfinder/PathFinder.h"

PathManager::PathManager()
{

}

PathManager::~PathManager()
{

}

void PathManager::ProcessMovement()
{
	if (WhetherRecalculPath())
	{
		Pathfinder::GetPathfinder()->ClearPathCost(pathIndexArr);
		Pathfinder::GetPathfinder()->FindPath(zombie->GetZombieLocation(), zombie->GetTargetLocation(), pathToTarget, pathIndexArr);
		InitializePathStatus();
	}
	FollowPath();
}

void PathManager::InitializePathStatus()
{
	pathIdx = 0;
	Pathfinder::GetPathfinder()->SetGridPassability(pathToTarget[pathIdx], false);
	nextPoint = Vector3D(pathToTarget[1].x, pathToTarget[1].y, zombie->GetZombieLocation().Z);
	nextDirection = (nextPoint - zombie->GetZombieLocation()).Normalize();
	zombie->SetNextGrid(nextPoint);
}

void PathManager::FollowPath()
{
	const float dist = nextPoint.GetDistance(zombie->GetZombieLocation());
	if (dist <= GRID_DIST)
	{
		if (pathIdx < pathToTarget.size())
		{
			if(pathIdx > 0) Pathfinder::GetPathfinder()->SetGridPassability(pathToTarget[pathIdx - 1], true);
			Pathfinder::GetPathfinder()->SetGridPassability(pathToTarget[pathIdx + 1], false);

			pathIdx++;
			nextPoint = Vector3D(pathToTarget[pathIdx].x, pathToTarget[pathIdx].y, zombie->GetZombieLocation().Z);
			nextDirection = (nextPoint - zombie->GetZombieLocation()).Normalize();
			zombie->SetNextGrid(nextPoint);
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
		// Ÿ�� ��ġ�� ����� ������ ��ġ �� �Ÿ��� 100 �̻��̸� ��� �ٽ� ���
		Vector3D targetLocation = zombie->GetTargetLocation();
		const float destToTarget = targetLocation.GetDistance(Vector3D(pathToTarget.back().x, pathToTarget.back().y, targetLocation.Z));
		if (destToTarget > 100.f)
		{
			return true;
		}
	}
	return false;
}

void PathManager::ClearPathStatus()
{

}
