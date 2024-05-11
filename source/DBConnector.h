#pragma once
#include <Windows.h>
#include <sqlext.h>
#include <string>
#include <vector>
#include "Structs/ItemInfo.h"
#include "Structs/GridPoint.h"
#include "Structs/PossessedItem.h"
#include "Structs/EquippedItem.h"
#include "Item/Item.h"
#pragma comment(lib, "odbc32.lib")

using namespace std;

const int ACCOUNT_CHAR_SIZE = 10;
const int ITEM_ID_CHAR_SIZE = 37;

enum class EQueryType
{
	SIGNUP,
	LOGIN,
};

class DBConnector final
{
public:

	static DBConnector* GetInstance()
	{
		static DBConnector instance;
		return &instance;
	}

	bool Initialize();

	bool Connect();

	void Close();

	bool PlayerLogin(const string& playerID, const string& pw);

	bool PlayerSignUp(const string& playerID, const string& pw);

	bool ExcuteQuery(const string& playerID, const string& pw, EQueryType queryType);

	void DeleteAllPlayerInventory(const string& playerID);

	void DeleteAllPlayerEquipment(const string& playerID);

	void SavePlayerInventory(const string& playerID, const string& itemID, int itemKey, int itemQuantity, bool isRotated, GridPoint addedPoint);

	void SavePlayerEquipment(const string& playerID, const string& itemID, int itemKey, int slotNumber);

	bool GetPlayerInventory(const string& playerID, vector<PossessedItem>& possessedItems);

	bool GetPlayerEquipment(const string& playerID, vector<EquippedItem>& equippedItems);

private:

	DBConnector() = default;
	~DBConnector() {}
	DBConnector(const DBConnector&)				= delete; // 복사 생성자 호출 방지
	DBConnector& operator=(const DBConnector&)	= delete; // 대입 연산자 호출 방지
	DBConnector(DBConnector&&)					= delete; // 이동 생성자 호출 방지
	DBConnector& operator=(DBConnector&&)		= delete; // 이동 대입 연산자 호출 방지

	// 에러가 발생하면, 에러코드를 통해 어떤 에러인지 알려줌
	void ErrorDisplay(RETCODE retCode);

private:

	SQLHENV		henv;
	SQLHDBC		hdbc;
	SQLHSTMT	hstmt;

	// 쿼리 관련 변수
	bool isPrimaryKeyError;

};