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
	param1 = SQL_NTS, param2 = SQL_NTS;
	SQLRETURN retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, id.length(), 0, (char*)id.c_str(), 0, &param1);
	retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, pw.length(), 0, (char*)pw.c_str(), 0, &param2);

	// 쿼리 실행 
	retcode = SQLExecDirect(hstmt, (wchar_t*)query.c_str(), SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		SQLCloseCursor(hstmt);
		return true;
	}
	else
	{
		ErrorDisplay(retcode);
		if (isPrimaryKeyError)
			isPrimaryKeyError = false;
	}

	SQLCloseCursor(hstmt);
	return false;
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
