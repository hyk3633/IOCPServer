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

	/* 스테이트 객체에서 호출 하는 함수 */

	void SetZombieState(ZombieState* newState);

	bool IsTargetSetted();

	/* path manager에서 호출하는 함수 */

	void AddMovement(const Vector3D& direction, const Vector3D& dest);

	/* getter, setter */

	inline void SetZombieInfo(const ZombieInfo& info) { zombieInfo = info; }

	inline ZombieInfo& GetZombieInfo() { return zombieInfo; }

	inline void SetZombieLocation(const Vector3D& location) { zombieInfo.characterInfo.location = location; }

	inline Vector3D GetZombieLocation() { return zombieInfo.characterInfo.location; }

	inline PathManager* GetPathManager() { return pathManager.get(); }
	
	inline void SetTarget(CharacterInfo* info) { targetInfo = info; }

	inline Vector3D GetTargetLocation() { return targetInfo->location; }

	inline float GetSpeed() const { return speed; }

	inline EZombieState GetStateEnum() const { return zombieInfo.state; }

	void SetSpeed(const Vector3D& dest);

	void SetPath(const vector<Pos>& path);

	inline void SetTargetNumber(const int number) { zombieInfo.targetNumber = number; }

private:

	ZombieState* zombieState;

	ZombieInfo zombieInfo;

	unique_ptr<PathManager> pathManager;

	CharacterInfo* targetInfo;

	float speed = 50.f;

};