#include "JsonComponent.h"

using namespace std;
using namespace rapidjson;

static string itemInfoFile("D:\\UE5Projects\\IOCPServer\\Json\\ItemInfo.json");

static string playerInfoFile("D:\\UE5Projects\\IOCPServer\\Json\\PlayerInfo.json");

static string concreteInfoName("ConcreteInfo");

void JsonComponent::Initialize()
{
    ifstream ifs(itemInfoFile);
    if (!ifs.is_open()) {
        cout << "[Error] : No json file.\n";
        return;
    }

    string json((istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>()));

    doc = Document(kObjectType);
    doc.Parse(json.c_str());
}

void JsonComponent::GetPlayerInfo(PlayerInfo& playerInfo)
{
    ifstream ifs(playerInfoFile);
    if (!ifs.is_open()) {
        cout << "[Error] : No json file.\n";
        return;
    }

    string json((istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>()));

    rapidjson::Document playerInfoDoc = Document(kObjectType);
    playerInfoDoc.Parse(json.c_str());

    Value& playerInfoValue = playerInfoDoc.GetObject();
    playerInfo.health   = playerInfoValue["Health"].GetInt();
    playerInfo.stamina   = playerInfoValue["Stamina"].GetInt();
    playerInfo.row      = playerInfoValue["Row"].GetInt();
    playerInfo.column   = playerInfoValue["Column"].GetInt();
}

void JsonComponent::GetItemCommonInfo(const int itemKey, ItemInfo& itemInfo)
{
    Value& item = doc[to_string(itemKey).c_str()];
    SaveItemCommonInfoToStruct(item, itemInfo);
}

void JsonComponent::GetItemConcreteInfo(const int itemKey, EItemMainType itemType, std::stringstream& stream)
{
    Value& item = doc[to_string(itemKey).c_str()];
    switch (itemType)
    {
        case EItemMainType::MeleeWeapon:
        {
            WeaponInfo info;
            SaveItemConcreteInfo(item, info);
            stream << info;
            break;
        }
        case EItemMainType::RangedWeapon:
        {
            RangedWeaponInfo info;
            SaveItemConcreteInfo(item, info);
            stream << info;
            break;
        }
        case EItemMainType::RecoveryItem:
        {
            RecoveryItemInfo info;
            SaveItemConcreteInfo(item, info);
            stream << info;
            break;
        }
        case EItemMainType::AmmoItem:
        {
            AmmoItemInfo info;
            SaveItemConcreteInfo(item, info);
            stream << info;
            break;
        }
        case EItemMainType::ArmourItem:
        {
            ArmourItemInfo info;
            SaveItemConcreteInfo(item, info);
            stream << info;
            break;
        }
    }
}

void JsonComponent::SaveItemCommonInfoToStruct(rapidjson::Value& valueObj, ItemInfo& itemInfo)
{
    itemInfo.itemKey       = valueObj["ItemKey"].GetInt();
    itemInfo.itemName      = valueObj["ItemName"].GetString();
    itemInfo.itemType      = static_cast<EItemMainType>(valueObj["ItemType"].GetInt());
    itemInfo.itemGridSize  = GridPoint{ valueObj["ItemGridSize"]["X"].GetInt(), valueObj["ItemGridSize"]["Y"].GetInt() };
    itemInfo.quantity      = valueObj["Quantity"].GetInt();
    itemInfo.isConsumable  = valueObj["IsConsumable"].GetInt();
}

void JsonComponent::SaveItemConcreteInfo(rapidjson::Value& valueObj, WeaponInfo& itemInfo)
{
    itemInfo.attackPower    = valueObj[concreteInfoName.c_str()]["AttackPower"].GetDouble();
    itemInfo.weaponType     = static_cast<EWeaponType>(valueObj[concreteInfoName.c_str()]["WeaponType"].GetInt());
}

void JsonComponent::SaveItemConcreteInfo(rapidjson::Value& valueObj, RangedWeaponInfo& itemInfo)
{
    itemInfo.attackPower    = valueObj[concreteInfoName.c_str()]["AttackPower"].GetDouble();
    itemInfo.weaponType     = static_cast<EWeaponType>(valueObj[concreteInfoName.c_str()]["WeaponType"].GetInt());
    itemInfo.magazine       = valueObj[concreteInfoName.c_str()]["Magazine"].GetInt();
    itemInfo.ammoType = valueObj[concreteInfoName.c_str()]["AmmoType"].GetInt();
}

void JsonComponent::SaveItemConcreteInfo(rapidjson::Value& valueObj, RecoveryItemInfo& itemInfo)
{
    itemInfo.recoveryAmount =  valueObj[concreteInfoName.c_str()]["RecoveryAmount"].GetInt();
    itemInfo.usingSpeed =      valueObj[concreteInfoName.c_str()]["UsingSpeed"].GetDouble();
}

void JsonComponent::SaveItemConcreteInfo(rapidjson::Value& valueObj, AmmoItemInfo& itemInfo)
{
    itemInfo.ammoType =    valueObj[concreteInfoName.c_str()]["AmmoType"].GetInt();
}

void JsonComponent::SaveItemConcreteInfo(rapidjson::Value& valueObj, ArmourItemInfo& itemInfo)
{
    itemInfo.defensePower =     valueObj[concreteInfoName.c_str()]["DefensePower"].GetDouble();
    itemInfo.armourSlot =       valueObj[concreteInfoName.c_str()]["ArmourSlot"].GetInt();
}
