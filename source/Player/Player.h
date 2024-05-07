#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include "../Character/Character.h"
#include "../Enums/WrestleState.h"
#include "../Structs/GridPoint.h"
#include "../Structs/PlayerItems.h"
#include "../Item/Item.h"

typedef void(*WrestlingCallback)(int);

typedef void(*PlayerDeadCallback)(int);

struct PossessedItem
{
	PossessedItem() : itemID(-1), addedPoint(), isEquipped(false), slotNumber(-1) {}
	PossessedItem(const int id, const GridPoint& point) : itemID(id), addedPoint(point), isEquipped(false), slotNumber(-1) {}
	int itemID;
	GridPoint addedPoint;
	bool isEquipped;
	int slotNumber;

	void Equip(const int number)
	{
		isEquipped = true;
		slotNumber = number;
	}

	void UnEquip()
	{
		isEquipped = false;
		slotNumber = -1;
	}
};

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

	bool UpdateItemGridPoint(shared_ptr<Item> item, const int itemID, GridPoint& pointToAdd, const bool isRotated);

	bool TryAddItem(shared_ptr<Item> item, const int itemID);

	void AddItem(shared_ptr<Item> item, const GridPoint& topLeftPoint, const GridPoint& gridSize, const int itemID);

	void RemoveItemInInventory(shared_ptr<Item> item, const int itemID);

	void RemoveItem(const int itemID);

	bool IsPlayerHasItem(const int itemID);

	void PlayerEquipItem(shared_ptr<Item> item, const int itemID, const int slotNumber);

	const unordered_map<int, PossessedItem>& GetInventoryStatus() const;

protected:

	bool IsRoomAvailable(const GridPoint& topLeftPoint, const GridPoint& gridSize, const int itemID);

	bool IsPitInInventory(const int ySize, const int xSize);

	bool IsGridValid(const int y, const int x);

	void PrintInventoryStatus();

private:

	string playerID;

	bool isInGameMap = false;

	Vector3D velocity;

	// 클라이언트 전송용 데이터

	bool isSuccessToBlocking = false;

	EWrestleState wrestleState = EWrestleState::ABLE;

	float wrestleWaitTime = 10.f;

	float wrestleWaitElapsedTime = 0.f;

	WrestlingCallback wrestlingCb = nullptr;

	WrestlingCallback playerDeadCb = nullptr;

	double currentRatencyStart;

	int zombieNumberWrestleWith;

	// 스탯

	float health = 100;

	float maxHealth = 100;

	bool isDead = 0;

	// 인벤토리

	unordered_map<int, PossessedItem> items;

	vector<vector<int>> inventoryGrids;

	// 하드코딩 하지말고 입력받기
	int columns = 6;

	int rows = 15;

};