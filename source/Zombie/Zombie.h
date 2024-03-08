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

	void ProcessMovement();

	/* path manager���� ȣ���ϴ� �Լ� */

	void AddMovement(const Vector3D& direction, const Vector3D& dest);

	/* getter, setter */

	inline PathManager* GetPathManager() { return pathManager.get(); }

	inline void SetZombieInfo(const ZombieInfo& info) { zombieInfo = info; }

	inline ZombieInfo& GetZombieInfo() { return zombieInfo; }

	inline void SetZombieLocation(const Vector3D& location) { zombieInfo.location = location; }

	inline Vector3D GetZombieLocation() { return zombieInfo.location; }
	
	inline void SetTarget(CharacterInfo* info) { targetInfo = info; }

	inline void SetTargetNumber(const int number) { zombieInfo.targetNumber = number; }

	inline Vector3D GetTargetLocation() { return targetInfo->location; }

	inline EZombieState GetStateEnum() const { return zombieInfo.state; }

	inline void SetNextGrid(const Vector3D& nextLocation) { zombieInfo.nextLocation = nextLocation; }

private:

	ZombieState* zombieState;

	ZombieInfo zombieInfo;

	unique_ptr<PathManager> pathManager;

	CharacterInfo* targetInfo;

	float speed = 100.f;

	float interval = 0.016f;

};