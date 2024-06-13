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
#include "../Structs/PlayerStatus.h"
#include "../Structs/PlayerInfo.h"
#include "../Item/Item.h"

typedef void(*WrestlingCallback)(int);

typedef void(*PlayerDeadCallback)(int);

typedef void(*PlayerHealthChangedCallback)(int, float, bool);

class Player : public Character, public std::enable_shared_from_this<Player>
{
public:

	Player() = default;

	Player(const int num, const PlayerInfo& playerInfo);

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

	void RegisterPlayerHealthChangedCallback(PlayerHealthChangedCallback phc);

	virtual void SerializeData(std::ostream& stream) override;

	void DeserializeData(std::istream& stream);

	void Waiting();

	virtual void TakeDamage(const float damage) override;

	void Heal(const float healingAmount);

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

	PlayerStatus GetPlayerStatus() const;

	void SerializePlayerInitialInfo(stringstream& sendStream);

	void ArmWeapon(const string& weaponID);

	void DisarmWeapon();

	inline string GetArmedWeaponID() const { return armedWeaponID; }

	inline float GetHealth() const { return health; }

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

	float wrestleWaitTime = 30.f;

	float wrestleWaitElapsedTime = 0.f;

	WrestlingCallback wrestlingCb = nullptr;

	WrestlingCallback playerDeadCb = nullptr;

	PlayerHealthChangedCallback playerHealthChangedCb = nullptr;

	int zombieNumberWrestleWith;

	// 스탯

	float health;

	float maxHealth;

	bool isDead = 0;

	float stamina;

	// 인벤토리

	// key : itemID, value : addedPoint
	unordered_map<string, GridPoint> possessedItems;

	// key : itemID, value : slotNumber
	unordered_map<string, int> equippedItems;

	vector<vector<int>> inventoryGrids;

	unordered_map<string, int> itemIDNumberMap;

	string armedWeaponID;

	int lastIDNumber = 0;

	int columns;

	int rows;

};