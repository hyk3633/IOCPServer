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

	/* 스레드에서 호출 하는 함수 */
	
	void ChangeState();

	void Update();

	void AllZombieInfoBitOn();

	/* 스테이트 객체에서 호출 하는 함수 */

	void SetZombieState(ZombieState* newState);

	bool IsTargetSetted();

	void ProcessMovement();

	/* path manager에서 호출하는 함수 */

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

	/* 타겟 관련 함수 */

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