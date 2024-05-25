#include "DBConnector.h"
#include <iostream>

bool DBConnector::Initialize()
{
	setlocale(LC_ALL, "korean");
	//std::wcout.imbue(std::locale("korean"));

	SQLRETURN retcode;

	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// Set the ODBC version environment attribute  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			{
				cout << "[DB] : Successfully Initialized!" << endl;
				return true;
			}
			else ErrorDisplay(retcode);
		}
		else ErrorDisplay(retcode);
	}
	else ErrorDisplay(retcode);

	return false;
}

bool DBConnector::Connect()
{
	SQLRETURN retcode;
	wstring odbc = L"game_odbc";
	//wstring id = L"id";
	//wstring pwd = L"password";

	SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (void*)5, 0);

	// Windows 통합인증 경우
	retcode = SQLConnect(hdbc, (wchar_t*)odbc.c_str(), SQL_NTS, nullptr, 0, nullptr, 0);

	// Allocate statement handle  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
		cout << "[DB] : Connection complete!" << endl;
	}
	else
	{
		ErrorDisplay(retcode);
		cout << "[DB Error] : Failed to connect to DB." << endl;
		return false;
	}

	return true;
}

void DBConnector::Close()
{
	SQLCancel(hstmt);
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

bool DBConnector::PlayerLogin(const string& id, const string& pw)
{
	return ExcuteQuery(id, pw, EQueryType::LOGIN);
}

bool DBConnector::PlayerSignUp(const string& id, const string& pw)
{
	return ExcuteQuery(id, pw, EQueryType::SIGNUP);
}

bool DBConnector::ExcuteQuery(const string& id, const string& pw, EQueryType queryType)
{
	////TEST
	//const_cast<string&>(id) = "hyk5805";
	//const_cast<string&>(pw) = "hyk5805";

	// 쿼리문
	wstring query;
	if (queryType == EQueryType::LOGIN)
	{
		query = L"SELECT * FROM IOCPTest.dbo.AccountInfo WHERE (PlayerID = ? AND Password = ?)";
	}
	else if (queryType == EQueryType::SIGNUP)
	{
		query = L"INSERT INTO IOCPTest.dbo.AccountInfo(PlayerID, Password) VALUES (?, ?)";
	}
	else
	{
		cout << "[DB Error] : Invalid query type." << endl;
		return false;
	}

	// 쿼리 매개변수
	SQLLEN param1 = SQL_NTS, param2 = SQL_NTS;
	SQLRETURN retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, id.length(), 0, (char*)id.c_str(), 0, &param1);
	retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, pw.length(), 0, (char*)pw.c_str(), 0, &param2);

	// 쿼리 실행 
	retcode = SQLExecDirect(hstmt, (wchar_t*)query.c_str(), SQL_NTS);

	if (retcode == SQL_SUCCESS)
	{
		char ret_id[ACCOUNT_CHAR_SIZE], ret_pw[ACCOUNT_CHAR_SIZE];
		ZeroMemory(&ret_id, ACCOUNT_CHAR_SIZE);
		ZeroMemory(&ret_pw, ACCOUNT_CHAR_SIZE);
		SQLLEN slen_id = 0, slen_pw = 0;

		retcode = SQLBindCol(hstmt, 1, SQL_C_CHAR, ret_id, ACCOUNT_CHAR_SIZE, &slen_id);
		retcode = SQLBindCol(hstmt, 2, SQL_C_CHAR, ret_pw, ACCOUNT_CHAR_SIZE, &slen_pw);

		do {
			retcode = SQLFetch(hstmt);
		} while (retcode != SQL_NO_DATA);

		SQLCloseCursor(hstmt);

		if (strlen(ret_id) == 0 || strlen(ret_pw) == 0) 
			return false;
		else
			return true;
	}
	else
	{
		ErrorDisplay(retcode);
		if (isPrimaryKeyError)
			isPrimaryKeyError = false;
	}

	return false;
}

void DBConnector::DeleteAllPlayerInventory(const string& playerID)
{
	wstring deleteQuery = L"DELETE FROM IOCPTest.dbo.PlayerInventory WHERE (PlayerID = ?)";
	SQLLEN param = SQL_NTS;
	SQLRETURN retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, playerID.length(), 0, (char*)playerID.c_str(), 0, &param);
	retcode = SQLExecDirect(hstmt, (wchar_t*)deleteQuery.c_str(), SQL_NTS);
	if (retcode != SQL_SUCCESS)
	{
		ErrorDisplay(retcode);
		if (isPrimaryKeyError)
			isPrimaryKeyError = false;
	}
	SQLCloseCursor(hstmt);
}

void DBConnector::DeleteAllPlayerEquipment(const string& playerID)
{
	wstring deleteQuery = L"DELETE FROM IOCPTest.dbo.PlayerEquipment WHERE (PlayerID = ?)";
	SQLLEN param = SQL_NTS;
	SQLRETURN retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, playerID.length(), 0, (char*)playerID.c_str(), 0, &param);
	retcode = SQLExecDirect(hstmt, (wchar_t*)deleteQuery.c_str(), SQL_NTS);
	if (retcode != SQL_SUCCESS)
	{
		ErrorDisplay(retcode);
		if (isPrimaryKeyError)
			isPrimaryKeyError = false;
	}
	SQLCloseCursor(hstmt);
}

void DBConnector::SavePlayerInventory(const string& playerID, const string& itemID, int itemKey, int itemQuantity, bool isRotated, GridPoint addedPoint)
{
	wstring query = L"INSERT INTO IOCPTest.dbo.PlayerInventory(PlayerID, ItemID, ItemKey, Quantity, IsRotated, TopLeftX, TopLeftY) VALUES (?, ?, ?, ?, ?, ?, ?)";

	SQLRETURN retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, playerID.length(), 0, (char*)playerID.c_str(), 0, NULL);
	retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_C_CHAR, itemID.length(), 0, (char*)itemID.c_str(), 0, NULL);
	retcode = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemKey, 0, NULL);
	retcode = SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemQuantity, 0, NULL);
	retcode = SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &isRotated, 0, NULL);
	retcode = SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &addedPoint.x, 0, NULL);
	retcode = SQLBindParameter(hstmt, 7, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &addedPoint.y, 0, NULL);

	retcode = SQLExecDirect(hstmt, (wchar_t*)query.c_str(), SQL_NTS);

	if (retcode != SQL_SUCCESS)
	{
		ErrorDisplay(retcode);
		if (isPrimaryKeyError)
			isPrimaryKeyError = false;
	}
	SQLCloseCursor(hstmt);
}

void DBConnector::SavePlayerEquipment(const string& playerID, const string& itemID, int itemKey, int slotNumber)
{
	wstring query = L"INSERT INTO IOCPTest.dbo.PlayerEquipment(PlayerID, ItemID, ItemKey, SlotNumber) VALUES (?, ?, ?, ?)";

	SQLRETURN retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, playerID.length(), 0, (char*)playerID.c_str(), 0, NULL);
	retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_C_CHAR, itemID.length(), 0, (char*)itemID.c_str(), 0, NULL);
	retcode = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemKey, 0, NULL);
	retcode = SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &slotNumber, 0, NULL);

	retcode = SQLExecDirect(hstmt, (wchar_t*)query.c_str(), SQL_NTS);

	if (retcode != SQL_SUCCESS)
	{
		ErrorDisplay(retcode);
		if (isPrimaryKeyError)
			isPrimaryKeyError = false;
	}
	SQLCloseCursor(hstmt);
}

bool DBConnector::GetPlayerInventory(const string& playerID, vector<PossessedItem>& possessedItems)
{
	wstring query = L"SELECT ItemID, ItemKey, Quantity, IsRotated, TopLeftX, TopLeftY FROM IOCPTest.dbo.PlayerInventory WHERE (PlayerID = ?)";
	SQLLEN param = SQL_NTS;
	SQLRETURN retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, playerID.length(), 0, (char*)playerID.c_str(), 0, &param);
	retcode = SQLExecDirect(hstmt, (wchar_t*)query.c_str(), SQL_NTS);

	PossessedItem possessedItem;

	if (retcode == SQL_SUCCESS)
	{
		char itemID[ITEM_ID_CHAR_SIZE];
		ZeroMemory(&itemID, ITEM_ID_CHAR_SIZE);
		SQLLEN len_itemID = 0, len_itemKey = SQL_NTS, len_quantity = SQL_NTS, 
			len_isRotated = SQL_NTS, len_topLeftX = SQL_NTS,	len_topLeftY = SQL_NTS;

		SQLBindCol(hstmt, 1, SQL_C_CHAR,  &itemID,						sizeof(itemID),						&len_itemID);
		SQLBindCol(hstmt, 2, SQL_INTEGER, &possessedItem.itemKey,		sizeof(possessedItem.itemKey),		&len_itemKey);
		SQLBindCol(hstmt, 3, SQL_INTEGER, &possessedItem.quantity,		sizeof(possessedItem.quantity),		&len_quantity);
		SQLBindCol(hstmt, 4, SQL_INTEGER, &possessedItem.isRotated,		sizeof(possessedItem.isRotated),	&len_isRotated);
		SQLBindCol(hstmt, 5, SQL_INTEGER, &possessedItem.topLeftX,		sizeof(possessedItem.topLeftX),		&len_topLeftX);
		SQLBindCol(hstmt, 6, SQL_INTEGER, &possessedItem.topLeftY,		sizeof(possessedItem.topLeftY),		&len_topLeftY);

		while (SQLFetch(hstmt) != SQL_NO_DATA)
		{
			possessedItem.itemID = itemID;
			possessedItems.push_back(possessedItem);
		}

		SQLCloseCursor(hstmt);
		return true;
	}
	else
	{
		ErrorDisplay(retcode);
		if (isPrimaryKeyError)
			isPrimaryKeyError = false;
		SQLCloseCursor(hstmt);
		return false;
	}
}

bool DBConnector::GetPlayerEquipment(const string& playerID, vector<EquippedItem>& equippedItems)
{
	wstring query = L"SELECT ItemID, ItemKey, SlotNumber FROM IOCPTest.dbo.PlayerEquipment WHERE (PlayerID = ?)";
	SQLLEN param = SQL_NTS;
	SQLRETURN retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, playerID.length(), 0, (char*)playerID.c_str(), 0, &param);
	retcode = SQLExecDirect(hstmt, (wchar_t*)query.c_str(), SQL_NTS);

	EquippedItem equippedItem;

	if (retcode == SQL_SUCCESS)
	{
		char itemID[ITEM_ID_CHAR_SIZE];
		ZeroMemory(&itemID, ITEM_ID_CHAR_SIZE);
		SQLLEN len_itemID = SQL_NTS, len_itemKey = SQL_NTS, len_slotNumber = SQL_NTS;

		SQLBindCol(hstmt, 1, SQL_C_CHAR,  &itemID,						sizeof(itemID),						&len_itemID);
		SQLBindCol(hstmt, 2, SQL_INTEGER, &equippedItem.itemKey,		sizeof(equippedItem.itemKey),		&len_itemKey);
		SQLBindCol(hstmt, 3, SQL_INTEGER, &equippedItem.slotNumber,		sizeof(equippedItem.slotNumber),	&len_slotNumber);

		while (SQLFetch(hstmt) != SQL_NO_DATA)
		{
			equippedItem.itemID = itemID;
			equippedItems.push_back(equippedItem);
		}
		SQLCloseCursor(hstmt);
		return true;
	}
	else
	{
		ErrorDisplay(retcode);
		if (isPrimaryKeyError)
			isPrimaryKeyError = false;
		SQLCloseCursor(hstmt);
		return false;
	}
}

void DBConnector::ErrorDisplay(RETCODE retCode)
{
	SQLSMALLINT rec = 0;
	SQLINTEGER  error;

	wchar_t wszMessage[1000] = { 0, };
	wchar_t wszState[SQL_SQLSTATE_SIZE + 1];

	if (retCode == SQL_INVALID_HANDLE)
	{
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}

	while (SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, ++rec, wszState, &error, wszMessage, (short)(sizeof(wszMessage) / sizeof(wchar_t)), (short*)nullptr) == SQL_SUCCESS)
	{
		if (wcsncmp(wszState, L"01004", 5))
		{
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, error);
		}
		if (error == 2627)
		{
			isPrimaryKeyError = true;
			return;
		}
	}
}
