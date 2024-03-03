#include "ChaseState.h"
#include "IdleState.h"
#include "WaitState.h"
#include "AttackState.h"
#include "GrabState.h"
#include "../Zombie.h"
#include "../PathManager.h"

void ChaseState::ChangeState(Zombie* zombie)
{
	// �÷��̾ wrestle ���¸� wait
	//zombie.SetZombieState(WaitState::GetInstance());

	// �÷��̾ wait ���¸� attack
	zombie->SetZombieState(AttackState::GetInstance());

	// �÷��̾ �� �� ���°� �ƴϸ� grab
	//zombie.SetZombieState(GrabState::GetInstance());
}

void ChaseState::Update(Zombie* zombie)
{
	if (zombie && zombie->IsTargetSetted())
	{
		Vector3D myLocation = zombie->GetZombieLocation();
		Vector3D TargetLocation = zombie->GetTargetLocation();
		const float distance = myLocation.GetDistance(TargetLocation);

		if (distance <= 100.f)
		{
			zombie->SetZombieState(AttackState::GetInstance());
		}
		else if (distance > 100.f && distance <= 1000.f)
		{
			PathManager* pathManager = zombie->GetPathManager();
			vector<Pos>& path = pathManager->GetPathToTarget();
			if (pathManager->WhetherRecalculPath())
			{
				GetPathfinder()->FindPath(myLocation, TargetLocation, path);
				if (path.size() == 0) return;
				pathManager->InitializePathStatus();
			}
			pathManager->FollowPath();
		}
		else
		{
			zombie->SetZombieState(IdleState::GetInstance());
		}
	}
}

EZombieState ChaseState::GetStateEnum()
{
	return EZombieState::CHASE;
}
