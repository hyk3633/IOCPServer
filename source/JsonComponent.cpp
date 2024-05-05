#include "JsonComponent.h"

using namespace std;
using namespace rapidjson;

static string file("D:\\UE5Projects\\IOCPServer\\Json\\ItemInfo.json");

void JsonComponent::Initialize()
{
    ifstream ifs(file);
    if (!ifs.is_open()) {
        cout << "[Error] : No json file.\n";
        return;
    }

    string json((istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>()));

    doc = Document(kObjectType);
    doc.Parse(json.c_str());
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
        case EItemMainType::RangedWeapon:
        {
            WeaponInfo info;
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
    }
}

void JsonComponent::SaveItemCommonInfoToStruct(rapidjson::Value& valueObj, ItemInfo& itemInfo)
{
    itemInfo.itemKey       = valueObj["ItemKey"].GetInt();
    itemInfo.itemName      = valueObj["ItemName"].GetString();
    itemInfo.itemType      = static_cast<EItemMainType>(valueObj["ItemType"].GetInt());
    itemInfo.itemGridSize  = GridPoint{ valueObj["ItemGridSize"]["X"].GetInt(), valueObj["ItemGridSize"]["Y"].GetInt() };
    itemInfo.count         = valueObj["Count"].GetInt();
    itemInfo.isConsumable  = valueObj["IsConsumable"].GetInt();
}

void JsonComponent::SaveItemConcreteInfo(rapidjson::Value& valueObj, WeaponInfo& itemInfo)
{
    itemInfo.attackPower    = valueObj["WeaponType"].GetDouble();
    itemInfo.weaponType     = static_cast<EWeaponType>(valueObj["WeaponType"].GetInt());
}

void JsonComponent::SaveItemConcreteInfo(rapidjson::Value& valueObj, RangedWeaponInfo& itemInfo)
{
    itemInfo.attackPower    = valueObj["WeaponType"].GetDouble();
    itemInfo.weaponType     = static_cast<EWeaponType>(valueObj["WeaponType"].GetInt());
    itemInfo.fireRate       = valueObj["RangedWeapon"]["FireRate"].GetDouble();
    itemInfo.recoil         = valueObj["RangedWeapon"]["Recoil"].GetDouble();
    itemInfo.magazine       = valueObj["RangedWeapon"]["Magazine"].GetInt();
    itemInfo.reloadingSpeed = valueObj["RangedWeapon"]["ReloadingSpeed"].GetDouble();
}

void JsonComponent::SaveItemConcreteInfo(rapidjson::Value& valueObj, RecoveryItemInfo& itemInfo)
{
    itemInfo.recoveryAmount =  valueObj["RecoveryItem"]["RecoveryAmount"].GetInt();
    itemInfo.usingSpeed =      valueObj["RecoveryItem"]["UsingSpeed"].GetDouble();
}

void JsonComponent::SaveItemConcreteInfo(rapidjson::Value& valueObj, AmmoItemInfo& itemInfo)
{
    itemInfo.ammoType =    valueObj["AmmoItem"]["AmmoType"].GetInt();
}
