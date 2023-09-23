#include <fstream>
#include <ranges>
#include <rogue/Components/Buffs.h>
#include <rogue/ItemDatabase.h>
#include <rogue/JSON.h>


namespace rogue {

template <typename BuffType, typename... RequiredComps>
static std::shared_ptr<ApplyBuffItemEffect<BuffType, RequiredComps...>>
makeApplyBuffItemEffect(const BuffType &Buff) {
  return std::make_shared<ApplyBuffItemEffect<BuffType, RequiredComps...>>(
      Buff);
}

static std::shared_ptr<ItemEffect> createEffect(const rapidjson::Value &V) {
  static const std::map<std::string, std::function<std::shared_ptr<ItemEffect>(
                                         const rapidjson::Value &)>>
      Factories = {
          {"health_item_effect",
           [](const auto &V) {
             const auto HealthValue = V["health_value"].GetDouble();
             return std::make_shared<HealItemEffect>(HealthValue);
           }},
          {"damage_item_effect",
           [](const auto &V) {
             const auto DamageValue = V["damage_value"].GetDouble();
             return std::make_shared<DamageItemEffect>(DamageValue);
           }},
          {"poison_debuff_comp",
           [](const auto &V) {
             PoisonDebuffComp Buff;
             Buff.ReduceAmount = V["reduce_amount"].GetDouble();
             Buff.TicksLeft = V["ticks"].GetUint();
             return makeApplyBuffItemEffect<PoisonDebuffComp, HealthComp>(Buff);
           }},
          {"health_regen_buff_comp",
           [](const auto &V) {
             HealthRegenBuffComp Buff;
             Buff.RegenAmount = V["regen_amount"].GetDouble();
             Buff.TicksLeft = V["ticks"].GetUint();
             return makeApplyBuffItemEffect<HealthRegenBuffComp, HealthComp>(
                 Buff);
           }},
          {"bleeding_debuff_comp",
           [](const auto &V) {
             BleedingDebuffComp Buff;
             Buff.ReduceAmount = V["reduce_amount"].GetDouble();
             Buff.TicksLeft = V["ticks"].GetUint();
             return makeApplyBuffItemEffect<BleedingDebuffComp, HealthComp>(
                 Buff);
           }},
          {"stats_buff_comp",
           [](const auto &V) {
             StatPoints P;
             P.Str = V["stats"]["str"].GetInt();
             P.Dex = V["stats"]["dex"].GetInt();
             P.Int = V["stats"]["int"].GetInt();
             P.Vit = V["stats"]["vit"].GetInt();
             StatsBuffComp Buff{{1U}, P};
             return makeApplyBuffItemEffect<StatsBuffComp, StatsComp>(Buff);
           }},
      };

  const auto EffectType = V["type"].GetString();
  const auto &Factory = Factories.at(EffectType);

  return Factory(V);
}

ItemType ItemDatabase::getItemType(const std::string &ItemType) {
  if (ItemType == "consumable") {
    return ItemType::Consumable;
  }
  if (ItemType == "weapon") {
    return ItemType::Weapon;
  }
  if (ItemType == "crafting") {
    return ItemType::Crafting;
  }
  throw std::runtime_error("Unknown item type: " + std::string(ItemType));
  return ItemType::None;
}

CapabilityFlags
ItemDatabase::getCapabilityFlag(const std::string &CapabilityFlagStr) {
  if (CapabilityFlagStr == "use_on") {
    return CapabilityFlags::UseOn;
  }
  if (CapabilityFlagStr == "equipment") {
    return CapabilityFlags::Equipment;
  }
  throw std::runtime_error("Unknown capability: " + CapabilityFlagStr);
  return CapabilityFlags::None;
}

ItemDatabase ItemDatabase::load(const std::filesystem::path &ItemDbConfig) {
  ItemDatabase DB;

  const auto SchemaPath = ItemDbConfig.parent_path() / "item_db_schema.json";
  auto [DocStr, Doc] = loadJSON(ItemDbConfig, &SchemaPath);

  // Create effects
  std::map<std::string, std::shared_ptr<ItemEffect>> Effects;
  const auto &EffectsJson = Doc["item_effects"].GetObject();
  for (const auto &[K, V] : EffectsJson) {
    const auto EffectName = std::string(K.GetString());
    // Check if already registered
    if (Effects.count(EffectName) != 0) {
      throw std::runtime_error("Duplicate item effect: " + EffectName);
    }
    auto Effect = createEffect(V);
    Effects.emplace(EffectName, Effect);
  }

  // Create item prototypes
  int ItemId = 0;
  const auto &ItemProtosJson = Doc["item_prototypes"].GetArray();
  for (const auto &ItemProtoJson : ItemProtosJson) {
    const auto Name = std::string(ItemProtoJson["name"].GetString());
    const auto Description = ItemProtoJson["description"].GetString();
    const auto MaxStackSize = ItemProtoJson["max_stack_size"].GetInt();

    ItemType ItType = ItemType::None;
    for (const auto &ItemType : ItemProtoJson["types"].GetArray()) {
      const auto ItemTypeStr = ItemType.GetString();
      ItType |= getItemType(ItemTypeStr);
    }

    std::vector<ItemPrototype::EffectInfo> EffectInfos;
    for (const auto &CapJson : ItemProtoJson["capabilities"].GetArray()) {
      const auto &CapInfo = CapJson.GetObject();
      const auto Flag = getCapabilityFlag(CapInfo["type"].GetString());
      const auto Effect = Effects.at(CapJson["effect"].GetString());
      EffectInfos.push_back({Flag, Effect});
    }

    ItemPrototype Proto(ItemId++, Name, Description, ItType, MaxStackSize,
                        std::move(EffectInfos));
    DB.addItemProto(Proto);
  }

  return DB;
}

void ItemDatabase::addItemProto(const ItemPrototype &ItemProto) {
  // FIXME ID not yet taken
  assert(ItemProtos.count(ItemProto.ItemId) == 0);
  ItemProtos.emplace(ItemProto.ItemId, ItemProto);
}

Item ItemDatabase::createItem(int ItemId, int StackSize) const {
  auto It = ItemProtos.find(ItemId);
  if (It == ItemProtos.end()) {
    throw std::out_of_range("Unknown item id: " + std::to_string(ItemId));
  }
  return Item(It->second, StackSize);
}

} // namespace rogue