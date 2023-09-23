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

static StatPoints parseStatPoints(const rapidjson::Value &V) {
  StatPoints P;
  P.Str = V["str"].GetInt();
  P.Dex = V["dex"].GetInt();
  P.Int = V["int"].GetInt();
  P.Vit = V["vit"].GetInt();
  return P;
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
             StatPoints P = parseStatPoints(V["stats"]);
             StatsBuffComp Buff;
             Buff.Bonus = P;
             return makeApplyBuffItemEffect<StatsBuffComp, StatsComp>(Buff);
           }},
      };

  const auto EffectType = V["type"].GetString();
  const auto It = Factories.find(EffectType);
  if (It == Factories.end()) {
    throw std::out_of_range("Unknown item effect: " + std::string(EffectType));
  }

  return It->second(V);
}

static std::shared_ptr<ItemSpecialization>
createSpecialization(const rapidjson::Value &V) {
  static std::map<std::string,
                  std::function<std::shared_ptr<ItemSpecialization>(
                      const rapidjson::Value &)>>
      Factories = {
          {"stats_buff_comp_spec",
           [](const auto &V) {
             StatsBuffSpecialization SBS;
             SBS.MinPoints = V["min_points"].GetInt();
             SBS.MaxPoints = V["max_points"].GetInt();
             return std::make_unique<StatsBuffSpecialization>(SBS);
           }},
      };

  const auto EffectType = V["type"].GetString();
  const auto It = Factories.find(EffectType);
  if (It == Factories.end()) {
    throw std::out_of_range("Unknown item specialization: " +
                            std::string(EffectType));
  }
  return It->second(V);
}

ItemType ItemDatabase::getItemType(const std::string &Type) {
  static const std::map<std::string, ItemType> ItemTypes = {
      {"none", ItemType::None}, // Keep top
      {"ring", ItemType::Ring}, //
      {"amulet", ItemType::Amulet},
      {"helmet", ItemType::Helmet},
      {"chest_plate", ItemType::ChestPlate},
      {"pants", ItemType::Pants},
      {"boots", ItemType::Boots},
      {"weapon", ItemType::Weapon},
      {"off_hand", ItemType::OffHand},
      {"generic", ItemType::Generic},
      {"consumable", ItemType::Consumable},
      {"quest", ItemType::Quest},
      {"crafting", ItemType::Crafting},
  };
  if (const auto It = ItemTypes.find(Type); It != ItemTypes.end()) {
    return It->second;
  }
  throw std::runtime_error("Unknown item type: " + std::string(Type));
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

  // Create specializations
  std::map<std::string, std::shared_ptr<ItemSpecialization>> Specializations;
  const auto &SpecializationsJson = Doc["item_specializations"].GetObject();
  for (const auto &[K, V] : SpecializationsJson) {
    const auto SpecName = std::string(K.GetString());
    // Check if already registered
    if (Specializations.count(SpecName) != 0) {
      throw std::runtime_error("Duplicate item specialization: " + SpecName);
    }
    auto Spec = createSpecialization(V);
    Specializations.emplace(SpecName, Spec);
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

    std::unique_ptr<ItemSpecializations> Specialization;
    if (ItemProtoJson.HasMember("specializations")) {
      Specialization = std::make_unique<ItemSpecializations>();
      for (const auto &SpecJson : ItemProtoJson["specializations"].GetArray()) {
        const auto &SpecInfo = SpecJson.GetObject();
        const auto Flags = getCapabilityFlag(SpecInfo["type"].GetString());
        const auto Spec =
            Specializations.at(SpecInfo["specialization"].GetString());
        Specialization->addSpecialization(Flags, Spec);
      }
    }

    ItemPrototype Proto(ItemId++, Name, Description, ItType, MaxStackSize,
                        std::move(EffectInfos));
    DB.addItemProto(Proto, Specialization.get());
  }

  return DB;
}

void ItemDatabase::addItemProto(const ItemPrototype &ItemProto,
                                const ItemSpecializations *ItemSpec) {
  // FIXME ID not yet taken
  assert(ItemProtos.count(ItemProto.ItemId) == 0);
  ItemProtos.emplace(ItemProto.ItemId, ItemProto);
  if (ItemSpec) {
    ItemSpecs.emplace(ItemProto.ItemId, *ItemSpec);
  }
}

Item ItemDatabase::createItem(int ItemId, int StackSize) const {
  auto It = ItemProtos.find(ItemId);
  if (It == ItemProtos.end()) {
    throw std::out_of_range("Unknown item id: " + std::to_string(ItemId));
  }
  // Actualize the specialization
  std::shared_ptr<ItemPrototype> Spec = nullptr;
  if (auto SpecIt = ItemSpecs.find(ItemId); SpecIt != ItemSpecs.end()) {
    Spec = SpecIt->second.actualize(It->second);
  }
  return Item(It->second, StackSize, Spec);
}

int ItemDatabase::getRandomItemId() const {
  const auto Idx = rand() % ItemProtos.size();
  const auto It = std::next(ItemProtos.begin(), Idx);
  return It->first;
}

} // namespace rogue