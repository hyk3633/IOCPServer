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
		zombie->StartPathfindingTimer();

		Vector3D targetLocation;
		if (zombie->GetTargetLocation(targetLocation))
		{
			Pathfinder::GetPathfinder()->FindPath(zombie->GetLocation(), targetLocation, pathToTarget, pathIndexArr);
			if (pathToTarget.size() <= 1)
			{
				zombie->ClearPathfindingTimer();
				zombie->ChangeState();
			}
			else
			{
				InitializePathStatus();
			}
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
		if (pathIdx > 0 && pathIdx < pathToTarget.size())
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
	zombie->AddMovement(nextDirection, nextPoint, walkSpeed);
}

bool PathManager::WhetherRecalculPath()
{
	if (zombie->GetAblePathfinding() == false)
		return false;

	if (pathToTarget.size() == 0 || pathIdx >= pathToTarget.size())
	{
		return true;
	}
	else
	{
		// Ÿ�� ��ġ�� ����� ������ ��ġ �� �Ÿ��� 70 �̻��̸� ��� �ٽ� ���
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
	Pathfinder::GetPathfinder()->ClearPathCost(pathIndexArr);
	if (pathIdx + 1 < pathToTarget.size())
	{
		Pathfinder::GetPathfinder()->SetGridPassability(pathToTarget[pathIdx + 1], true);
		Pathfinder::GetPathfinder()->SetGridPassability(pathToTarget[pathIdx], true);
	}
	else if (pathIdx < pathToTarget.size())
	{
		Pathfinder::GetPathfinder()->SetGridPassability(pathToTarget[pathIdx], true);
	}
	pathToTarget.clear();
	pathIndexArr.clear();
}
