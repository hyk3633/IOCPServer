#pragma once

#include "State/ZombieState.h"
#include "PathManager.h"
#include "../CharacterInfo.h"

using namespace std;

class PathManager;

class Zombie
{
public:

	Zombie();

	~Zombie() = default;

	/* �����忡�� ȣ�� �ϴ� �Լ� */
	
	void ChangeState();

	void Update();

	/* ������Ʈ ��ü���� ȣ�� �ϴ� �Լ� */

	void SetZombieState(ZombieState* newState);

	bool IsTargetSetted();

	/* path manager���� ȣ���ϴ� �Լ� */

	void AddMovement(const Vector3D direction);

	/* getter, setter */

	inline void SetZombieInfo(const ZombieInfo& info) { zombieInfo = info; }

	inline ZombieInfo& GetZombieInfo() { return zombieInfo; }

	inline Vector3D GetZombieLocation() { return zombieInfo.characterInfo.location; }

	inline PathManager* GetPathManager() { return pathManager.get(); }
	
	inline void SetTarget(CharacterInfo* info) { targetInfo = info; }

	inline Vector3D GetTargetLocation() { return targetInfo->location; }

	inline float GetSpeed() const { return speed; }

	inline EZombieState GetStateEnum() const { return zombieInfo.state; }

private:

	ZombieState* zombieState;

	ZombieInfo zombieInfo;

	unique_ptr<PathManager> pathManager;

	CharacterInfo* targetInfo;

	float speed = 40.f;

};