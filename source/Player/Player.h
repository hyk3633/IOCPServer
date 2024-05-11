#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include "../Character/Character.h"
#include "../Enums/WrestleState.h"
#include "../Structs/GridPoint.h"
#include "../Structs/PossessedItem.h"
#include "../Structs/EquippedItem.h"
#include "../Item/Item.h"

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

	bool UpdateItemGridPoint(shared_ptr<Item> item, const string& itemID, GridPoint& pointToAdd, const bool isRotated);

	bool TryAddItem(shared_ptr<Item> item, const string& itemID);

	bool TryAddItemAt(shared_ptr<Item> item, const string& itemID, GridPoint& pointToAdd);

	void AddItem(const GridPoint& topLeftPoint, const GridPoint& gridSize, const string& itemID);

	void RemoveItemGrid(const GridPoint& addedPoint, const GridPoint& gridSize);

	bool IsPlayerHasItemInInventory(const string& itemID);

	bool IsPlayerHasItemInEquipment(const string& itemID);

	void RemoveItemInInventory(shared_ptr<Item> item, const string& itemID);

	void RemoveItemInEquipment(const string& itemID);

	void ItemEquipFromInventory(shared_ptr<Item> item, const string& itemID, const int slotNumber);

	void ItemEquipInitialize(const string& itemID, const int slotNumber);

	void PlayerUnEquipItem(shared_ptr<Item> item, const string& itemID);

	GridPoint GetItemsAddedPoint(const string& itemID);

	unordered_map<string, GridPoint>& GetPossessedItems();

	unordered_map<string, int>& GetEquippedItems();

protected:

	void AddItemToIDNumberMap(const string& itemID);

	bool IsRoomAvailable(const GridPoint& topLeftPoint, const GridPoint& gridSize, const int itemIDNumber = -1);

	bool IsPitInInventory(const int xSize, const int ySize);

	bool IsGridValid(const int x, const int y);

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

	// key : itemID, value : addedPoint
	unordered_map<string, GridPoint> possessedItems;

	// key : itemID, value : slotNumber
	unordered_map<string, int> equippedItems;

	vector<vector<int>> inventoryGrids;

	unordered_map<string, int> itemIDNumberMap;

	int lastIDNumber = 0;

	// 하드코딩 하지말고 입력받기
	int columns = 6;

	int rows = 15;

};