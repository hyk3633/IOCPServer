#pragma once
#include "../Character/Character.h"
#include "../Enums/WrestleState.h"
#include <vector>
#include <memory>

typedef void(*WrestlingCallback)(int);

typedef void(*PlayerDeadCallback)(int);

class Player : public Character, public std::enable_shared_from_this<Player>
{
public:

	Player(const int num);

	virtual ~Player() override;

	void InitializePlayerInfo();

	inline string GetPlayerID() const { return playerID; }

	void SetPlayerID(const string& id);

	inline bool IsPlayerInGameMap() const { return isInGameMap; }

	inline void PlayerInGameMap() { isInGameMap = true; }

	void WrestleStateOn();

	void WrestlStateOff();

	inline EWrestleState GetWrestleState() const { return wrestleState; }

	inline bool GetSuccessToBlocking() const { return isSuccessToBlocking; }

	void SetSuccessToBlocking(const bool isSuccess) { isSuccessToBlocking = isSuccess; }

	void RegisterWrestlingCallback(WrestlingCallback wc);

	void RegisterPlayerDeadCallback(PlayerDeadCallback pdc);

	virtual void SerializeData(std::ostream& stream) override;

	void DeserializeData(std::istream& stream);

	void Waiting();

	virtual void TakeDamage(const float damage) override;

	void SetZombieNumberWrestleWith(const int number);

	inline int GetZombieNumberWrestleWith() const { return zombieNumberWrestleWith; }

	inline bool GetIsDead() const { return isDead; }

protected:



private:

	string playerID;

	bool isInGameMap = false;

	Vector3D velocity;

	// Ŭ���̾�Ʈ ���ۿ� ������

	bool isSuccessToBlocking = false;

	EWrestleState wrestleState = EWrestleState::ABLE;

	float wrestleWaitTime = 10.f;

	float wrestleWaitElapsedTime = 0.f;

	WrestlingCallback wrestlingCb = nullptr;

	WrestlingCallback playerDeadCb = nullptr;

	double currentRatencyStart;

	int zombieNumberWrestleWith;

	// ����

	float health = 100;

	float maxHealth = 100;

	bool isDead = 0;

};