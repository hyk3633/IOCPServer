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

	virtual void SerializeData(std::ostream& stream) override;

	void DeserializeData(std::istream& stream);

	void SerializePlayerInitialInfo(stringstream& sendStream);

	// ������ �� ���� �ð� ���� ������ ���� ����
	void Waiting();

	void RegisterWrestlingCallback(WrestlingCallback wc);

	void RegisterPlayerDeadCallback(PlayerDeadCallback pdc);

	void RegisterPlayerHealthChangedCallback(PlayerHealthChangedCallback phc);

	void SetPlayerID(const string& id);

	inline string GetPlayerID() const { return playerID; }

	// �÷��̾ ���� �ʿ� �����ߴ���
	inline bool IsPlayerInGameMap() const { return isInGameMap; }

	inline void PlayerInGameMap() { isInGameMap = true; }

	void WrestleStateOn();

	void WrestleStateOff();

	inline EWrestleState GetWrestleState() const { return wrestleState; }

	// ���� ������ ��� ���� ���� ����
	void SetSuccessToBlocking(const bool isSuccess);

	inline bool GetSuccessToBlocking() const { return isSuccessToBlocking; }

	// �÷��̾�� ������ ���� ���� ��ȣ ��ȯ
	void SetZombieNumberWrestleWith(const int number);

	inline int GetZombieNumberWrestleWith() const { return zombieNumberWrestleWith; }

	virtual void TakeDamage(const float damage) override;

	void Heal(const float healingAmount);

	inline float GetHealth() const { return health; }

	inline bool GetIsDead() const { return isDead; }

	PlayerStatus GetPlayerStatus() const;

	/* ������ / �κ��丮 */

	void ArmWeapon(const string& weaponID);

	void DisarmWeapon();

	void RemoveItemInInventory(shared_ptr<Item> item, const string& itemID);

	void RemoveItemGrid(const GridPoint& addedPoint, const GridPoint& gridSize);

protected:

	void PrintInventoryStatus();

public:

	void RemoveItemInEquipment(const string& itemID);

	void ItemEquipFromInventory(shared_ptr<Item> item, const string& itemID, const int slotNumber);

	void ItemEquipInitialize(const string& itemID, const int slotNumber);

	void AddItemToIDNumberMap(const string& itemID);

	inline string GetArmedWeaponID() const { return armedWeaponID; }

protected:

	// �κ��丮�� ������ ��ġ ����
	bool UpdateItemGridPoint(shared_ptr<Item> item, const string& itemID, GridPoint& pointToAdd, const bool isRotated);

public:

	void AddItem(const GridPoint& topLeftPoint, const GridPoint& gridSize, const string& itemID);

protected:

	// �κ��丮�� �ش� ��ġ�� �������� �߰��� �� �ִ��� �˻�
	bool IsRoomAvailable(const GridPoint& topLeftPoint, const GridPoint& gridSize, const int itemIDNumber = -1);

public:

	bool TryAddItem(shared_ptr<Item> item, const string& itemID);

protected:

	// �κ��丮�� �߰��� �� �ִ� ũ�� ���� �˻�
	bool IsPitInInventory(const int xSize, const int ySize);

public:

	bool TryAddItemAt(shared_ptr<Item> item, const string& itemID, GridPoint& pointToAdd);

	bool IsPlayerHasItemInInventory(const string& itemID);

	bool IsPlayerHasItemInEquipment(const string& itemID);

	unordered_map<string, GridPoint>& GetPossessedItems();

	unordered_map<string, int>& GetEquippedItems();

public:

	GridPoint GetItemsAddedPoint(const string& itemID);

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

	PlayerHealthChangedCallback playerHealthChangedCb = nullptr;

	int zombieNumberWrestleWith;

	float pitch = 0.f;

	// ����

	float health;

	float maxHealth;

	bool isDead = 0;

	float stamina;

	// �κ��丮

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