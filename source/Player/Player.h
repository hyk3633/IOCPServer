#pragma once
#include "../Character/Character.h"
#include "../Enums/WrestleState.h"
#include "../Enums/PlayerInfoBitType.h"
#include <vector>
#include <memory>

typedef void(*WrestlingCallback)(int);

typedef void(*PlayerDeadCallback)(int);

class Player : public Character, public std::enable_shared_from_this<Player>
{
public:

	Player(const int num);

	virtual ~Player() override;

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

	void SerializeExtraData(std::ostream& stream);

	void SaveInfoToPacket(std::ostream& stream, const int bitType);

	void DeserializeData(std::istream& stream);

	void DeserializeExtraData(std::istream& stream);

	void ReceiveInfoToPacket(std::istream& stream, const int bitType);

	void Waiting();

	virtual void TakeDamage(const float damage) override;

	inline int GetRecvInfoBitMask() const { return recvInfoBitMask; }

	inline const std::vector<int>& GetZombiesInRange() const { return zombiesInRange; }

	inline const std::vector<int>& GetZombiesOutRange() const { return zombiesOutRange; }

	inline int GetZombieNumberAttackedBy() const { return zombieNumberAttackedBy; }

	void SetZombieNumberWrestleWith(const int number);

	inline int GetZombieNumberWrestleWith() const { return zombieNumberWrestleWith; }

protected:



private:

	string playerID;

	bool isInGameMap = false;

	int recvInfoBitMask = 0;

	Vector3D velocity;

	// Ŭ���̾�Ʈ ���ſ� ������

	std::vector<int> zombiesInRange, zombiesOutRange;

	bool isHit;

	int zombieNumberAttackedBy; // �� ���� ����

	int zombieNumberWrestleWith; // ��� ���� ����

	// Ŭ���̾�Ʈ ���ۿ� ������

	int sendInfoBitMask;

	bool isSuccessToBlocking;

	EWrestleState wrestleState = EWrestleState::ABLE;

	float wrestleWaitTime = 5.f;

	float wrestleWaitElapsedTime = 0.f;

	WrestlingCallback wrestlingCb = nullptr;

	WrestlingCallback playerDeadCb = nullptr;

	// ����

	float health = 100;

	float maxHealth = 100;

};