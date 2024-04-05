#include "PathManager.h"
#include "Zombie.h"
#include "State/ZombieState.h"
#include "../Player/Player.h"
#include "../Pathfinder/PathFinder.h"
#include "../Structs/Rotator.h"

using namespace std;

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
		Vector3D targetLocation;
		if (zombie->GetTargetLocation(targetLocation))
		{
			Pathfinder::GetPathfinder()->FindPath(zombie->GetLocation(), targetLocation, pathToTarget, pathIndexArr);
			InitializePathStatus();
		}
		else
		{
			zombie->ChangeState();
		}
	}
	FollowPath();
}

void PathManager::InitializePathStatus()
{
	pathIdx = 0;
	Pathfinder::GetPathfinder()->SetGridPassability(pathToTarget[pathIdx], false);
	nextPoint = Vector3D(pathToTarget[pathIdx + 1].x, pathToTarget[pathIdx + 1].y, zombie->GetLocation().Z);
	nextDirection = (nextPoint - zombie->GetLocation()).Normalize();
	zombie->SetNextGrid(nextPoint);
}

void PathManager::FollowPath()
{
	const float dist = nextPoint.GetDistance(zombie->GetLocation());
	if (dist <= GRID_DIST)
	{
		if (pathIdx > 0)
		{
			Pathfinder::GetPathfinder()->SetGridPassability(pathToTarget[pathIdx - 1], true);
		}
		if (pathIdx + 1 < pathToTarget.size())
		{
			Pathfinder::GetPathfinder()->SetGridPassability(pathToTarget[pathIdx + 1], false);

			pathIdx++;
			nextPoint = Vector3D(pathToTarget[pathIdx].x, pathToTarget[pathIdx].y, zombie->GetLocation().Z);
			nextDirection = (nextPoint - zombie->GetLocation()).Normalize();
			zombie->SetRotation(nextDirection.Rotation());
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
		// 타겟 위치와 경로의 마지막 위치 간 거리가 70 이상이면 경로 다시 계산
		Vector3D targetLocation;
		if (zombie->GetTargetLocation(targetLocation))
		{
			const float destToTarget = targetLocation.GetDistance(Vector3D(pathToTarget.back().x, pathToTarget.back().y, targetLocation.Z));
			if (destToTarget > 70.f)
			{
				return true;
			}
		}
	}
	return false;
}

void PathManager::ClearPathStatus()
{

}
