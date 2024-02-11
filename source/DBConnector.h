#pragma once
#include <Windows.h>
#include <sqlext.h>
#include <string>
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
	SQLLEN param1, param2;
	char ret_id[paramSize], ret_pw[paramSize];
	SQLLEN slen_id = 0, slen_pw = 0;
	bool isPrimaryKeyError;

};