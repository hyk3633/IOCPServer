#pragma once
#include "Item.h"
#include <unordered_map>
#include <sstream>
#include <mutex>

class ItemManager
{
public:

	ItemManager();
	~ItemManager() = default;

	void SetItemStateToActivated(const int itemNumber);

	void SetItemStateToDeactivated(const int itemNumber);

	void SaveItemInfoToPacket(std::ostream& stream);

	// 지정된 위치에 아이템 생성하는 함수

private:

	std::unordered_map<int, Item> itemMap;

	std::unordered_map<int, Item*> deactivatedItemMap;

	std::unordered_map<int, Item*> activatedItemMap;

	std::mutex itemMutex;

};

/*

아이템의 상태
활성 : 필드에 존재하는 상태
Acquired : 플레이어가 획득하여 필드에는 없는 상태
비활성 : 아이템이 사용되어 다음 아이템 스폰 시 사용될 대기 상태

처음 -> 서버에서 지정된 위치에 아이템 스폰 -> 아이템 위치 지정 툴로 스폰할 위치 받아오기
플레이어가 아이템 획득 알림 -> 아이템 매니저에 해당 아이템을 비활성 처리 하고 플레이어의 소유 아이템에 추가(활성->획득)
플레이어가 아이템 사용 -> 아이템 매니저에 해당 아이템 상태 변경 요청(획득->비활성)
플레이어가 아이템 버림 -> (획득->활성)
새로운 아이템 스폰 -> 아이템매니저에 요청 (비활성->활성)

활성 상태인 아이템의 위치를 유지해서 플레이어가 아이템을 획득했을 때 
서버에서 획득 가능한 위치인지 검증

*/