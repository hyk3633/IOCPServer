#include "Player.h"
#include <iostream>
using namespace std;

#define EMPTY -1

Player::Player(const int num, const PlayerInfo& playerInfo) : 
	Character(num), 
	maxHealth(playerInfo.health), 
	health(playerInfo.health), 
	stamina(playerInfo.stamina),
	rows(playerInfo.row), 
	columns(playerInfo.column)
{
	inventoryGrids = vector<vector<int>>(rows, vector<int>(columns, EMPTY));
}

Player::~Player()
{
}

void Player::InitializePlayerInfo()
{
	SetLocation(Vector3D{ 0, 0, 97.9f });
	SetRotation(Rotator{ 0, 0, 0 });
	velocity = Vector3D{ 0, 0, 0 };
	isSuccessToBlocking = false;
	wrestleState = EWrestleState::ABLE;
	wrestleWaitElapsedTime = 0.f;
	zombieNumberWrestleWith = -1;
	health = maxHealth;
	isDead = false;
}

void Player::SerializeData(ostream& stream)
{
	SerializeLocation(stream);
	SerializeRotation(stream);
	stream << velocity;
	stream << pitch << "\n";
}

void Player::DeserializeData(istream& stream)
{
	DeserializeLocation(stream);
	DeserializeRotation(stream);
	stream >> velocity;
	stream >> pitch;
}

void Player::SerializePlayerInitialInfo(stringstream& sendStream)
{
	sendStream << maxHealth << "\n";
	sendStream << stamina << "\n";
	sendStream << rows << "\n";
	sendStream << columns << "\n";
}

void Player::Waiting()
{
	if (wrestleState == EWrestleState::WAITING)
	{
		wrestleWaitElapsedTime += 0.016f;
		if (wrestleWaitElapsedTime >= wrestleWaitTime)
		{
			wrestleWaitElapsedTime = 0.f;
			wrestleState = EWrestleState::ABLE;
		}
	}
}

void Player::RegisterWrestlingCallback(WrestlingCallback wc)
{
	wrestlingCb = wc;
}

void Player::RegisterPlayerDeadCallback(PlayerDeadCallback pdc)
{
	playerDeadCb = pdc;
}

void Player::RegisterPlayerHealthChangedCallback(PlayerHealthChangedCallback phc)
{
	playerHealthChangedCb = phc;
}

void Player::SetPlayerID(const string& id)
{
	playerID = id;
}

void Player::WrestleStateOn()
{
	wrestleState = EWrestleState::WRESTLING;
	wrestlingCb(GetNumber());
}

void Player::WrestleStateOff()
{
	wrestleState = EWrestleState::WAITING;
}

void Player::SetSuccessToBlocking(const bool isSuccess)
{
	isSuccessToBlocking = isSuccess;
}

void Player::SetZombieNumberWrestleWith(const int number)
{
	zombieNumberWrestleWith = number;
}

void Player::TakeDamage(const float damage)
{
	health = max(health - damage, 0.f);
	playerHealthChangedCb(GetNumber(), health, true);
	if (health == 0.f)
	{
		// critical section
		isDead = true;
		playerDeadCb(GetNumber());
	}
	cout << "[Log] : 클라이언트 " << GetNumber() << " 체력 : " << health << "\n";
}

void Player::Heal(const float healingAmount)
{
	health = min(health + healingAmount, maxHealth);
	playerHealthChangedCb(GetNumber(), health, true);
}

PlayerStatus Player::GetPlayerStatus() const
{
	return { health };
}

void Player::ArmWeapon(const string& weaponID)
{
	armedWeaponID = weaponID;
}

void Player::DisarmWeapon()
{
	armedWeaponID = "";
}

void Player::RemoveItemInInventory(shared_ptr<Item> item, const string& itemID)
{
	RemoveItemGrid(possessedItems[itemID], item->itemInfo.itemGridSize);
	possessedItems.erase(itemID);
	itemIDNumberMap.erase(itemID);

	PrintInventoryStatus();
}

void Player::PrintInventoryStatus()
{
	for (int r = 0; r < rows; ++r)
	{
		for (int c = 0; c < columns; ++c)
		{
			if (inventoryGrids[r][c] == EMPTY)
				cout << "*" << " ";
			else
				cout << inventoryGrids[r][c] << " ";
		}
		cout << endl;
	}
	cout << endl;
}

void Player::RemoveItemGrid(const GridPoint& addedPoint, const GridPoint& gridSize)
{
	for (int r = addedPoint.y; r < addedPoint.y + gridSize.y; ++r)
	{
		for (int c = addedPoint.x; c < addedPoint.x + gridSize.x; ++c)
		{
			inventoryGrids[r][c] = EMPTY;
		}
	}
}

void Player::RemoveItemInEquipment(const string& itemID)
{
	equippedItems.erase(itemID);
	itemIDNumberMap.erase(itemID);
}

void Player::ItemEquipFromInventory(shared_ptr<Item> item, const string& itemID, const int slotNumber)
{
	RemoveItemGrid(possessedItems[itemID], item->itemInfo.itemGridSize);
	possessedItems.erase(itemID);
	equippedItems[itemID] = slotNumber;
}

void Player::ItemEquipInitialize(const string& itemID, const int slotNumber)
{
	AddItemToIDNumberMap(itemID);
	equippedItems[itemID] = slotNumber;
}

void Player::AddItemToIDNumberMap(const string& itemID)
{
	itemIDNumberMap[itemID] = lastIDNumber++;
}

bool Player::UpdateItemGridPoint(shared_ptr<Item> item, const string& itemID, GridPoint& pointToAdd, const bool isRotated)
{
	const GridPoint addedPoint = possessedItems[itemID];
	GridPoint gridSize = item->itemInfo.itemGridSize;

	if (isRotated != item->isRotated) 
		gridSize = { gridSize.y, gridSize.x };

	const int itemIDNumber = itemIDNumberMap[itemID];
	
	if (IsRoomAvailable(pointToAdd, gridSize, itemIDNumber))
	{
		if (isRotated != item->isRotated)
			gridSize = { gridSize.y, gridSize.x };

		RemoveItemGrid(addedPoint, gridSize);

		if (isRotated != item->isRotated)
			item->Rotate();

		AddItem(pointToAdd, item->itemInfo.itemGridSize, itemID);

		return true;
	}
	else
	{
		return false;
	}
}

void Player::AddItem(const GridPoint& topLeftPoint, const GridPoint& gridSize, const string& itemID)
{
	possessedItems[itemID] = topLeftPoint;
	AddItemToIDNumberMap(itemID);
	const int itemIDNumber = itemIDNumberMap[itemID];
	for (int r = topLeftPoint.y; r < topLeftPoint.y + gridSize.y; ++r)
	{
		for (int c = topLeftPoint.x; c < topLeftPoint.x + gridSize.x; ++c)
		{
			inventoryGrids[r][c] = itemIDNumber;
		}
	}
	PrintInventoryStatus();
}

bool Player::IsRoomAvailable(const GridPoint& topLeftPoint, const GridPoint& gridSize, const int itemIDNumber)
{
	for (int r = topLeftPoint.y; r < topLeftPoint.y + gridSize.y; ++r)
	{
		for (int c = topLeftPoint.x; c < topLeftPoint.x + gridSize.x; ++c)
		{
			if (itemIDNumber == -1 && inventoryGrids[r][c] != EMPTY)
				return false;
			else if (itemIDNumber != -1 && inventoryGrids[r][c] != EMPTY && inventoryGrids[r][c] != itemIDNumber)
				return false;
		}
	}
	return true;
}

bool Player::TryAddItem(shared_ptr<Item> item, const string& itemID)
{
	GridPoint gridSize = item->itemInfo.itemGridSize;
	for (int r = 0; r < rows; ++r)
	{
		for (int c = 0; c < columns; ++c)
		{
			if (IsPitInInventory(c + gridSize.x, r + gridSize.y) && IsRoomAvailable({ c, r }, gridSize))
			{
				AddItem({ c,r }, gridSize, itemID);
				return true;
			}
		}
	}

	gridSize = { gridSize.y, gridSize.x };
	for (int r = 0; r < rows; ++r)
	{
		for (int c = 0; c < columns; ++c)
		{
			if (IsPitInInventory(c + gridSize.y, r + gridSize.x) && IsRoomAvailable({ c, r }, gridSize))
			{
				item->Rotate();
				AddItem({ c, r}, gridSize, itemID);
				return true;
			}
		}
	}
	return false;
}

bool Player::IsPitInInventory(const int xSize, const int ySize)
{
	return (ySize > 0 && ySize <= rows && xSize > 0 && xSize <= columns);
}

bool Player::TryAddItemAt(shared_ptr<Item> item, const string& itemID, GridPoint& pointToAdd)
{
	if (IsRoomAvailable(pointToAdd, item->itemInfo.itemGridSize))
	{
		AddItem(pointToAdd, item->itemInfo.itemGridSize, itemID);
		return true;
	}
	else
	{
		return false;
	}
}

bool Player::IsPlayerHasItemInInventory(const string& itemID)
{
	if (possessedItems.count(itemID))
		return true;
	else
		return false;
}

bool Player::IsPlayerHasItemInEquipment(const string& itemID)
{
	if (equippedItems.find(itemID) != equippedItems.end())
		return true;
	else
		return false;
}

unordered_map<string, GridPoint>& Player::GetPossessedItems()
{
	return possessedItems;
}

unordered_map<string, int>& Player::GetEquippedItems()
{
	return equippedItems;
}

GridPoint Player::GetItemsAddedPoint(const string& itemID)
{
	return possessedItems[itemID];
}