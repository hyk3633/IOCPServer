#pragma once
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/prettywriter.h"
#include "Structs/ItemInfo.h"
#include "Structs/IntPoint.h"
#include <memory>
#include <iostream>
#include <fstream>
#include <string>

class JsonComponent
{
public:

	JsonComponent() = default;
	~JsonComponent() = default;

	void Initialize();

	std::shared_ptr<ItemInfo> GetItemInfo(const int itemKey);

	void SaveCommonInfo(rapidjson::Value& valueObj, ItemInfo* itemInfo);

	void SaveConcreteInfo(rapidjson::Value& valueObj, MeleeWeaponInfo* itemInfo);

	void SaveConcreteInfo(rapidjson::Value& valueObj, RangedWeaponInfo* itemInfo);

	void SaveConcreteInfo(rapidjson::Value& valueObj, RecoveryItemInfo* itemInfo);

	void SaveConcreteInfo(rapidjson::Value& valueObj, AmmoItemInfo* itemInfo);

private:

	rapidjson::Document doc;

};