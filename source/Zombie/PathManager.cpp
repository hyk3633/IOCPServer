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
	cout << "loc : " << nextPoint.X << " " << nextPoint.Y << " " << nextPoint.Z << "\n";
	zombie->SetSpeed(nextPoint);
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
			zombie->SetSpeed(nextPoint);
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

void PathManager::CorrectZombieLocation()
{
	const Vector3D correctedLocation = zombie->GetZombieLocation() + (nextPoint - zombie->GetZombieLocation()) * elapsedTime;
	zombie->SetZombieLocation(correctedLocation);
}
