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

	/* ������ ȣ�� �Լ� */

	// ������ �ֱ� ���� ȣ���ϴ� ���� ���� �Լ�
	void Update();

	void ChangeState();

	/* ������Ʈ ��ü ȣ�� �Լ� */

	void SetZombieState(ZombieState* newState);

	// InRangeMap�� �ִ� �÷��̾� �� ���� ����� �÷��̾� Ž��
	bool FindNearestPlayer();

	// ChaseState �� �̵� ��� ���� ��ġ ������Ʈ
	void ProcessMovement();

	// WaitState �� ������ �ð� ���� ���
	bool Waiting();

	/* path manager ȣ�� �Լ� */

	void AddMovement(const Vector3D& direction, const Vector3D& dest, const float speed);

	/* �÷��̾� ���� */

	// Ŭ���̾�Ʈ���� �÷��̾��� Sphere Collision�� ���� �������� ���
	// �ش� �÷��̾ ������ Ÿ�� �ĺ� �ʿ� �߰���
	void AddPlayerToInRangeMap(std::shared_ptr<Player> player);

	// �÷��̾ Ÿ�� �ĺ��� �ʿ��� ����
	void RemoveInRangePlayer(const int playerNumber);

	// Ÿ���� �����Ǿ� �ִ��� Ȯ���ϰ� ������ ����
	void CheckTargetAndCancelTargetting(const int playerNumber);

	/* ��� Ž�� ���� �Լ� */

	// ������ �ð� ���� ��� Ž�� ���
	void StartPathfindingTimer();

	void ClearPathfindingTimer();

	void UpdatePathfindingTime();

	void ClearStateStatus();

	/* ����ȭ �Լ� */

	virtual void SerializeData(std::ostream& stream) override;

	// ������ ��� ���� ������ ��Ŷ ���� �� ����ȭ�ϵ��� ��Ʈ����ŷ
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

	/* ���� �Լ� */

	// ���� ���� �� 2.5�� ���� 5�� ����
	void BitingAttackToTarget();

	/* ü�� �Լ� */

	virtual void TakeDamage(const float damage) override;

private:

	ZombieState* zombieState;

	EZombieState stateEnum = EZombieState::IDLE;

	std::unique_ptr<PathManager> pathManager;

	Vector3D nextLocation;

	std::weak_ptr<Player> targetWeakPtr;

	// ���� ��� �ĺ� ��
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

	// ����

	float health = 200;

	float maxHealth = 200;

	float attackPower = 30;

	ZombieHealthChangedCallback zombieHealthChangedCb;

	ZombieDeadCallback zombieDeadCb;

};