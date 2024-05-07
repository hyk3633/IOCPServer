#pragma once
#include <Windows.h>
#include <sqlext.h>
#include <string>
#include <vector>
#include "Structs/PlayerItems.h"
#pragma comment(lib, "odbc32.lib")

using namespace std;

const int paramSize = 10;

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

	bool PlayerLogin(const string& id, const string& pw);

	bool PlayerSignUp(const string& id, const string& pw);

	bool ExcuteQuery(const string& id, const string& pw, EQueryType queryType);

	void SavePlayersItemInfo(const string& id, PlayerItems& playersItem);

	bool PlayerHasItem(const string& id, const int itemID);

	void GetPlayersItems(const string& id, std::vector<PlayerItems>& playerItemsArr);

	void SavePlayerStatus(const string& id, vector<PlayerItems>& playerItems);

private:

	DBConnector() = default;
	~DBConnector() {}
	DBConnector(const DBConnector&)				= delete; // ���� ������ ȣ�� ����
	DBConnector& operator=(const DBConnector&)	= delete; // ���� ������ ȣ�� ����
	DBConnector(DBConnector&&)					= delete; // �̵� ������ ȣ�� ����
	DBConnector& operator=(DBConnector&&)		= delete; // �̵� ���� ������ ȣ�� ����

	// ������ �߻��ϸ�, �����ڵ带 ���� � �������� �˷���
	void ErrorDisplay(RETCODE retCode);

private:

	SQLHENV		henv;
	SQLHDBC		hdbc;
	SQLHSTMT	hstmt;

	// ���� ���� ����
	bool isPrimaryKeyError;

};