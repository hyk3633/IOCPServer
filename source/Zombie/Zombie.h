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

	void RegisterZombieDeadCallback(ZombieDeadCallback zdc);

	void RegisterZombieHealthChangedCallback(ZombieHealthChangedCallback zhc);

	/* 스레드 호출 함수 */

	// 스레드 주기 마다 호출하는 상태 갱신 함수
	void Update();

	void ChangeState();

	/* 스테이트 객체 호출 함수 */

	void SetZombieState(ZombieState* newState);

	// InRangeMap에 있는 플레이어 중 가장 가까운 플레이어 탐색
	bool FindNearestPlayer();

	// ChaseState 중 이동 경로 따라 위치 업데이트
	void ProcessMovement();

	// WaitState 중 지정한 시간 동안 대기
	bool Waiting();

	/* path manager 호출 함수 */

	void AddMovement(const Vector3D& direction, const Vector3D& dest, const float speed);

	/* 플레이어 감지 */

	// 클라이언트에서 플레이어의 Sphere Collision에 좀비가 오버랩된 경우
	// 해당 플레이어를 좀비의 타겟 후보 맵에 추가함
	void AddPlayerToInRangeMap(std::shared_ptr<Player> player);

	// 플레이어를 타겟 후보군 맵에서 제거
	void RemoveInRangePlayer(const int playerNumber);

	// 타겟이 설정되어 있는지 확인하고 있으면 리셋
	void CheckTargetAndCancelTargetting(const int playerNumber);

	/* 경로 탐색 간격 함수 */

	// 지정한 시간 동안 경로 탐색 대기
	void StartPathfindingTimer();

	void ClearPathfindingTimer();

	void UpdatePathfindingTime();

	void ClearStateStatus();

	/* 직렬화 함수 */

	virtual void SerializeData(std::ostream& stream) override;

	// 좀비의 모든 상태 정보를 패킷 전송 시 직렬화하도록 비트마스킹
	void AllZombieInfoBitOn();

	bool IsTargetValid();

protected:

	void SaveInfoToPacket(std::ostream& stream, const int bitType);

	void MaskToInfoBit(const ZIBT bitType);

	void InitializeInfo();

public:
	
	/* getter, setter */

	EZombieState GetStateEnum() const { return stateEnum; }
	
	PathManager* GetPathManager();

	void SetRotation(const Rotator& rotation);

	void SetNextGrid(const Vector3D& nextLoc);

	inline int GetSendInfoBit() const { return sendInfoBitMask; }

	void SetTargetPlayer(std::shared_ptr<Player> player);

	std::shared_ptr<Player> GetTargetPlayer() const;

	bool GetTargetLocation(Vector3D& location);

	inline float GetHealth() const { return health; }

	inline float GetAttackPower() const { return attackPower; }

	inline bool GetAblePathfinding() const { return isAblePathfinding; }

	/* 공격 함수 */

	// 물기 공격 시 2.5초 동안 5번 공격
	void BitingAttackToTarget();

	/* 체력 함수 */

	virtual void TakeDamage(const float damage) override;

private:

	ZombieState* zombieState;

	EZombieState stateEnum = EZombieState::IDLE;

	std::unique_ptr<PathManager> pathManager;

	Vector3D nextLocation;

	std::weak_ptr<Player> targetWeakPtr;

	// 공격 대상 후보 맵
	std::map<int, std::weak_ptr<Player>> inRangePlayerMap;

	int sendInfoBitMask = 0;

	float speed = 100.f;

	float interval = 0.016f;

	float waitingTime = 2.5f;

	float elapsedWaitingTime = 0.f;

	int damageCount = 0;

	bool isAblePathfinding = true;

	bool bStartPathfindingCooldownTime = false;

	float elapsedPathfindingTime = 0.f;

	float pathfindingCooldownTime = 0.2f;

	// 스탯

	float health = 200;

	float maxHealth = 200;

	float attackPower = 30;

	ZombieHealthChangedCallback zombieHealthChangedCb;

	ZombieDeadCallback zombieDeadCb;

};