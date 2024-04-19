#pragma once
#include "Item.h"
#include "../Structs/ItemInfo.h"
#include <memory>
#include <unordered_map>
#include <sstream>
#include <mutex>

class JsonComponent;

class ItemManager
{
public:

	ItemManager();
	~ItemManager() = default;

	void SetItemStateToActivated(const int itemNumber);

	void SetItemStateToDeactivated(const int itemNumber);

	void SaveItemInfoToPacket(std::ostream& stream);

	// ������ ��ġ�� ������ �����ϴ� �Լ�

private:

	unique_ptr<JsonComponent> jsonComponent;

	std::unordered_map<int, shared_ptr<Item>> itemMap;

	std::unordered_map<int, weak_ptr<Item>> deactivatedItemMap;

	std::unordered_map<int, weak_ptr<Item>> activatedItemMap;

	std::mutex itemMutex;

};

/*

�������� ����
Ȱ�� : �ʵ忡 �����ϴ� ����
Acquired : �÷��̾ ȹ���Ͽ� �ʵ忡�� ���� ����
��Ȱ�� : �������� ���Ǿ� ���� ������ ���� �� ���� ��� ����

ó�� -> �������� ������ ��ġ�� ������ ���� -> ������ ��ġ ���� ���� ������ ��ġ �޾ƿ���
�÷��̾ ������ ȹ�� �˸� -> ������ �Ŵ����� �ش� �������� ��Ȱ�� ó�� �ϰ� �÷��̾��� ���� �����ۿ� �߰�(Ȱ��->ȹ��)
�÷��̾ ������ ��� -> ������ �Ŵ����� �ش� ������ ���� ���� ��û(ȹ��->��Ȱ��)
�÷��̾ ������ ���� -> (ȹ��->Ȱ��)
���ο� ������ ���� -> �����۸Ŵ����� ��û (��Ȱ��->Ȱ��)

Ȱ�� ������ �������� ��ġ�� �����ؼ� �÷��̾ �������� ȹ������ �� 
�������� ȹ�� ������ ��ġ���� ����

*/