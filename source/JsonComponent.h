#pragma once
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/prettywriter.h"
#include "Structs/ItemInfo.h"
#include "Structs/GridPoint.h"
#include "Structs/PlayerInfo.h"
#include "Enums/WeaponType.h"
#include "Enums/ItemType.h"
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

class JsonComponent
{
public:

	JsonComponent() = default;
	~JsonComponent() = default;

	void Initialize();

	void GetPlayerInfo(PlayerInfo& playerInfo);

	void GetItemCommonInfo(const int itemKey, ItemInfo& itemInfo);

	void GetItemConcreteInfo(const int itemKey, EItemMainType itemType, std::stringstream& stream);

protected:

	void SaveItemCommonInfoToStruct(rapidjson::Value& valueObj, ItemInfo& itemInfo);

	void SaveItemConcreteInfo(rapidjson::Value& valueObj, WeaponInfo& itemInfo);
		 
	void SaveItemConcreteInfo(rapidjson::Value& valueObj, RangedWeaponInfo& itemInfo);
		 
	void SaveItemConcreteInfo(rapidjson::Value& valueObj, RecoveryItemInfo& itemInfo);
		 
	void SaveItemConcreteInfo(rapidjson::Value& valueObj, AmmoItemInfo& itemInfo);

private:

	rapidjson::Document doc;

};