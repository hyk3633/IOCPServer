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

	// ������ ��ġ�� ������ �����ϴ� �Լ�

private:

	std::unordered_map<int, Item> itemMap;

	std::unordered_map<int, Item*> deactivatedItemMap;

	std::unordered_map<int, Item*> activatedItemMap;

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