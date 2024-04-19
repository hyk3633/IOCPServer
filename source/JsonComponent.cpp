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

shared_ptr<ItemInfo> JsonComponent::GetItemInfo(const int itemKey)
{
    Value& item = doc["Items"][itemKey];
    ItemInfo* itemInfo;
    switch (item["ItemKey"].GetInt())
    {
        case 0:
        {
            itemInfo = new MeleeWeaponInfo();
            SaveCommonInfo(item, itemInfo);
            SaveConcreteInfo(item, static_cast<MeleeWeaponInfo*>(itemInfo));
            break;
        }
        case 1:
        {
            itemInfo = new RangedWeaponInfo();
            SaveCommonInfo(item, itemInfo);
            SaveConcreteInfo(item, static_cast<RangedWeaponInfo*>(itemInfo));
            break;
        }
        case 2:
        {
            itemInfo = new RecoveryItemInfo();
            SaveCommonInfo(item, itemInfo);
            SaveConcreteInfo(item, static_cast<RecoveryItemInfo*>(itemInfo));
            break;
        }
        case 3:
        {
            itemInfo = new AmmoItemInfo();
            SaveCommonInfo(item, itemInfo);
            SaveConcreteInfo(item, static_cast<AmmoItemInfo*>(itemInfo));
            break;
        }
        default:
        {
            return nullptr;
        }
    }
    return shared_ptr<ItemInfo>(itemInfo);
}

void JsonComponent::SaveCommonInfo(rapidjson::Value& valueObj, ItemInfo* itemInfo)
{
    itemInfo->itemKey = valueObj["ItemKey"].GetInt();
    itemInfo->itemName = valueObj["ItemName"].GetString();
    itemInfo->itemType = static_cast<EItemMainType>(valueObj["ItemType"].GetInt());
    itemInfo->itemGridSize = IntPoint{ valueObj["ItemGridSize"]["X"].GetInt(), valueObj["ItemGridSize"]["Y"].GetInt() };
}

void JsonComponent::SaveConcreteInfo(rapidjson::Value& valueObj, MeleeWeaponInfo* itemInfo)
{
    itemInfo->attackPower = valueObj["MeleeWeapon"]["AttackPower"].GetDouble();
    itemInfo->attackSpeed = valueObj["MeleeWeapon"]["AttackSpeed"].GetDouble();
}

void JsonComponent::SaveConcreteInfo(rapidjson::Value& valueObj, RangedWeaponInfo* itemInfo)
{
    itemInfo->attackPower =     valueObj["RangedWeapon"]["AttackPower"].GetDouble();
    itemInfo->fireRate =        valueObj["RangedWeapon"]["FireRate"].GetDouble();
    itemInfo->recoil =          valueObj["RangedWeapon"]["Recoil"].GetDouble();
    itemInfo->magazine =        valueObj["RangedWeapon"]["Magazine"].GetInt();
    itemInfo->reloadingSpeed =  valueObj["RangedWeapon"]["ReloadingSpeed"].GetDouble();
}

void JsonComponent::SaveConcreteInfo(rapidjson::Value& valueObj, RecoveryItemInfo* itemInfo)
{
    itemInfo->recoveryAmount =  valueObj["RecoveryItem"]["RecoveryAmount"].GetInt();
    itemInfo->usingSpeed =      valueObj["RecoveryItem"]["UsingSpeed"].GetDouble();
}

void JsonComponent::SaveConcreteInfo(rapidjson::Value& valueObj, AmmoItemInfo* itemInfo)
{
    itemInfo->ammoType =    valueObj["AmmoItem"]["AmmoType"].GetInt();
    itemInfo->amount =      valueObj["AmmoItem"]["Amount"].GetInt();
}
