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

	void AllZombieInfoBitOn();

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

	inline EZombieState GetStateEnum() const { return zombieInfo.state; }

	inline void SetZombieLocation(const Vector3D& location) { zombieInfo.location = location; }

	inline Vector3D GetZombieLocation() const { return zombieInfo.location; }

	void SetZombieRotation(const Rotator& rotation);

	inline Rotator GetZombieRotation() const { return zombieInfo.rotation; }

	void SetNextGrid(const Vector3D& nextLocation);

	bool Waiting();

	/* Ÿ�� ���� �Լ� */

	inline void SetTarget(PlayerInfo* info) { targetInfo = info; }

	void SetTargetNumber(const int number);

	inline Vector3D GetTargetLocation() { return targetInfo->characterInfo.location; }

	inline Rotator GetTargetRotation() { return targetInfo->characterInfo.rotation; }

	inline EWrestleState GetTargetWrestleState() const { return targetInfo->wrestleState; }

	void SetTargetWrestleState(const EWrestleState state);

	inline bool GetTargetSuccessToBlock() const { return targetInfo->isSuccessToBlocking; }

protected:

	void MaskToInfoBit(const ZIBT bitType);

private:

	ZombieState* zombieState;

	ZombieInfo zombieInfo;

	unique_ptr<PathManager> pathManager;

	PlayerInfo* targetInfo;

	float speed = 100.f;

	float interval = 0.016f;

	float waitingTime = 1.25f;

	float elapsedWaitingTime = 0.f;

};