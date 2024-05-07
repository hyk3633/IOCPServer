#include "Player.h"
#include <iostream>
using namespace std;

#define EMPTY -1

Player::Player(const int num) : Character(num)
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

void Player::SetPlayerID(const string& id)
{
	playerID = id;
}

void Player::WrestleStateOn()
{
	wrestleState = EWrestleState::WRESTLING;
	wrestlingCb(GetNumber());
}

void Player::WrestlStateOff()
{
	wrestleState = EWrestleState::WAITING;
}

void Player::RegisterWrestlingCallback(WrestlingCallback wc)
{
	wrestlingCb = wc;
}

void Player::RegisterPlayerDeadCallback(PlayerDeadCallback pdc)
{
	playerDeadCb = pdc;
}

void Player::SerializeData(ostream& stream)
{
	stream << GetNumber() << "\n";
	SerializeLocation(stream);
	SerializeRotation(stream);
	stream << velocity;
	stream << currentRatencyStart << "\n";
}

void Player::DeserializeData(istream& stream)
{
	DeserializeLocation(stream);
	DeserializeRotation(stream);
	stream >> velocity;
	stream >> currentRatencyStart;
}

void Player::Waiting()
{
	if (wrestleState == EWrestleState::WAITING)
	{
		wrestleWaitElapsedTime += 0.2f;
		if (wrestleWaitElapsedTime >= wrestleWaitTime)
		{
			wrestleWaitElapsedTime = 0.f;
			wrestleState = EWrestleState::ABLE;
		}
	}
}

void Player::TakeDamage(const float damage)
{
	health = max(health - damage, 0.f);
	if (health == 0.f)
	{
		// critical section
		isDead = true;
		playerDeadCb(GetNumber());
	}
}

void Player::SetZombieNumberWrestleWith(const int number)
{
	zombieNumberWrestleWith = number;
}

bool Player::UpdateItemGridPoint(shared_ptr<Item> item, const int itemID, GridPoint& pointToAdd, const bool isRotated)
{
	const GridPoint addedPoint = item->gridPoint;
	GridPoint gridSize = item->itemInfo.itemGridSize;

	if (isRotated != item->isRotated) 
		gridSize = { gridSize.y, gridSize.x };
	
	if (IsRoomAvailable(pointToAdd, gridSize, itemID))
	{
		if (isRotated != item->isRotated)
			gridSize = { gridSize.y, gridSize.x };

		for (int r = addedPoint.y; r < addedPoint.y + gridSize.y; ++r)
		{
			for (int c = addedPoint.x; c < addedPoint.x + gridSize.x; ++c)
			{
				inventoryGrids[r][c] = EMPTY;
			}
		}

		if (isRotated != item->isRotated)
			item->Rotate();
		AddItem(item, pointToAdd, item->itemInfo.itemGridSize, itemID);

		return true;
	}
	else
	{
		return false;
	}
}

bool Player::TryAddItem(shared_ptr<Item> item, const int itemID)
{
	GridPoint gridSize = item->itemInfo.itemGridSize;
	for (int r = 0; r < rows; ++r)
	{
		for (int c = 0; c < columns; ++c)
		{
			if (IsPitInInventory(c + gridSize.x, r + gridSize.y) && IsRoomAvailable({ c, r }, gridSize, itemID))
			{
				AddItem(item, { c,r }, gridSize, itemID);
				return true;
			}
		}
	}
	for (int r = 0; r < rows; ++r)
	{
		for (int c = 0; c < columns; ++c)
		{
			if (IsPitInInventory(c + gridSize.y, r + gridSize.x) && IsRoomAvailable({ c, r }, {gridSize.y, gridSize.x}, itemID))
			{
				AddItem(item, { c, r}, { gridSize.y, gridSize.x }, itemID);
				item->isRotated = true;
				return true;
			}
		}
	}
	return false;
}

void Player::AddItem(shared_ptr<Item> item, const GridPoint& topLeftPoint, const GridPoint& gridSize, const int itemID)
{
	if (items.find(itemID) != items.end())
	{
		items[itemID].UnEquip();
	}
	else
	{
		items.emplace(itemID, PossessedItem(itemID, topLeftPoint));
	}

	item->gridPoint = topLeftPoint;
	for (int r = topLeftPoint.y; r < topLeftPoint.y + gridSize.y; ++r)
	{
		for (int c = topLeftPoint.x; c < topLeftPoint.x + gridSize.x; ++c)
		{
			inventoryGrids[r][c] = itemID;
		}
	}
	PrintInventoryStatus();
}

void Player::RemoveItemInInventory(shared_ptr<Item> item, const int itemID)
{
	const GridPoint addedPoint = item->gridPoint;
	const GridPoint gridSize = item->itemInfo.itemGridSize;
	for (int r = addedPoint.y; r < addedPoint.y + gridSize.y; ++r)
	{
		for (int c = addedPoint.x; c < addedPoint.x + gridSize.x; ++c)
		{
			inventoryGrids[r][c] = EMPTY;
		}
	}
	item->gridPoint = { -1,-1 };
}

void Player::RemoveItem(const int itemID)
{
	items.erase(itemID);
}

bool Player::IsPlayerHasItem(const int itemID)
{
	if (items.find(itemID) != items.end())
		return true;
	else
		return false;
}

void Player::PlayerEquipItem(shared_ptr<Item> item, const int itemID, const int slotNumber)
{
	RemoveItemInInventory(item, itemID);
	items[itemID].Equip(slotNumber);
}

const unordered_map<int, PossessedItem>& Player::GetInventoryStatus() const
{
	return items;
}

bool Player::IsRoomAvailable(const GridPoint& topLeftPoint, const GridPoint& gridSize, const int itemID)
{
	for (int r = topLeftPoint.y; r < topLeftPoint.y + gridSize.y; ++r)
	{
		for (int c = topLeftPoint.x; c < topLeftPoint.x + gridSize.x; ++c)
		{
			if (inventoryGrids[r][c] != EMPTY && inventoryGrids[r][c] != itemID)
				return false;
		}
	}
	return true;
}

bool Player::IsPitInInventory(const int ySize, const int xSize)
{
	return (ySize > 0 && ySize <= rows && xSize > 0 && xSize <= columns);
}

bool Player::IsGridValid(const int y, const int x)
{
	return (y >= 0 && y < rows && x >= 0 && x < columns);
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


