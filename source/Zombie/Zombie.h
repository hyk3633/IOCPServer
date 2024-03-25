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

class Zombie : public Character, public std::enable_shared_from_this<Zombie>
{
public:

	Zombie(const int num);

	virtual ~Zombie() override;

	/* �����忡�� ȣ�� �ϴ� �Լ� */
	
	void ChangeState();

	void Update();

	void AddPlayerToInRangeMap(std::shared_ptr<Player> player);

	void RemoveInRangePlayer(const int playerNumber);

	void AllZombieInfoBitOn();

	void RegisterZombieDeadCallback(ZombieDeadCallback zdc);

	/* ������Ʈ ��ü���� ȣ�� �ϴ� �Լ� */

	void SetZombieState(ZombieState* newState);

	bool CheckNearestPlayer();

	bool IsTargetSet();

	void ProcessMovement();

	bool Waiting();

	/* path manager���� ȣ���ϴ� �Լ� */

	void AddMovement(const Vector3D& direction, const Vector3D& dest);

	/* getter, setter */

	PathManager* GetPathManager();

	EZombieState GetStateEnum() const { return stateEnum; }

	void SetRotation(const Rotator& rotation);

	void SetNextGrid(const Vector3D& nextLoc);

	inline int GetSendInfoBit() const { return sendInfoBitMask; }

	void Activate();

	void Deactivate();

	inline bool GetIsActive() const { return isActive; }

	/* Ÿ�� ���� �Լ� */

	void SetTargetPlayer(std::shared_ptr<Player> player);

	std::shared_ptr<Player> GetTargetPlayer() const { return targetPlayer; }

	/* ����ȭ */

	virtual void SerializeData(std::ostream& stream) override;

	/*  */

	virtual void TakeDamage(const float damage) override;

protected:

	void SaveInfoToPacket(std::ostream& stream, const int bitType);

	void MaskToInfoBit(const ZIBT bitType);

private:

	ZombieState* zombieState;

	EZombieState stateEnum = EZombieState::IDLE;

	std::unique_ptr<PathManager> pathManager;

	Vector3D nextLocation;

	std::shared_ptr<Player> targetPlayer;

	std::map<int, std::shared_ptr<Player>> inRangePlayerMap;

	int sendInfoBitMask;

	float speed = 100.f;

	float interval = 0.016f;

	float waitingTime = 1.25f;

	float elapsedWaitingTime = 0.f;

	ZombieDeadCallback zombieDeadCb;

	bool isActive = false;

	// ���� ����

	float health = 100;

	float maxHealth = 100;

};