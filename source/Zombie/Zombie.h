#pragma once

#include "../Character/Character.h"
#include "../Enums/ZombieStateEnum.h"
#include <map>
#include <memory>

class ZombieState;
class PathManager;
class Player;

enum class EZombieInfoBitType
{
	Location,
	Rotation,
	State,
	TargetNumber,
	NextLocation,
	MAX
};

typedef EZombieInfoBitType ZIBT;

typedef void(*ZombieDeadCallback)(int);

typedef void(*ZombieHealthChangedCallback)(int, float, bool);

class Zombie : public Character, public std::enable_shared_from_this<Zombie>
{
public:

	Zombie(const int num);

	virtual ~Zombie() override;

	/* 스레드에서 호출 하는 함수 */
	
	void ChangeState();

	void Update();

	void AddPlayerToInRangeMap(std::shared_ptr<Player> player);

	void RemoveInRangePlayer(const int playerNumber);

	void AllZombieInfoBitOn();

	void RegisterZombieDeadCallback(ZombieDeadCallback zdc);

	void RegisterZombieHealthChangedCallback(ZombieHealthChangedCallback zhc);

	/* 스테이트 객체에서 호출 하는 함수 */

	void SetZombieState(ZombieState* newState);

	bool FindNearestPlayer();

	void ProcessMovement();

	bool Waiting();

	/* path manager에서 호출하는 함수 */

	bool IsTargetValid();

	void AddMovement(const Vector3D& direction, const Vector3D& dest, const float speed);

	/* getter, setter */

	PathManager* GetPathManager();

	EZombieState GetStateEnum() const { return stateEnum; }

	void SetRotation(const Rotator& rotation);

	void SetNextGrid(const Vector3D& nextLoc);

	inline int GetSendInfoBit() const { return sendInfoBitMask; }

	void Activate();

	void Deactivate();

	inline bool GetIsActive() const { return isActive; }

	/* 타겟 관련 함수 */

	void SetTargetPlayer(std::shared_ptr<Player> player);

	std::shared_ptr<Player> GetTargetPlayer() const;

	bool GetTargetLocation(Vector3D& location);

	void CheckTargetAndCancelTargetting(const int playerNumber);

	/* 직렬화 */

	virtual void SerializeData(std::ostream& stream) override;

	/*  */

	virtual void TakeDamage(const float damage) override;

	inline float GetHealth() const { return health; }

	inline float GetAttackPower() const { return attackPower; }

	void BitingAttackToTarget();

protected:

	void SaveInfoToPacket(std::ostream& stream, const int bitType);

	void MaskToInfoBit(const ZIBT bitType);

	void InitializeInfo();
	
public:

	void ClearStateStatus();

private:

	ZombieState* zombieState;

	EZombieState stateEnum = EZombieState::IDLE;

	std::unique_ptr<PathManager> pathManager;

	Vector3D nextLocation;

	std::weak_ptr<Player> targetWeakPtr;

	std::map<int, std::weak_ptr<Player>> inRangePlayerMap;

	int sendInfoBitMask = 0;

	float speed = 100.f;

	float interval = 0.016f;

	float waitingTime = 2.5f;

	float elapsedWaitingTime = 0.f;

	ZombieDeadCallback zombieDeadCb;

	bool isActive = false;

	int damageCount = 0;

	// 스탯

	float health = 200;

	float maxHealth = 200;

	float attackPower = 30;

	ZombieHealthChangedCallback zombieHealthChangedCb;

};