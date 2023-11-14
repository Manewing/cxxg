#include <fstream>
#include <ranges>
#include <rogue/Components/Buffs.h>
#include <rogue/Components/Combat.h>
#include <rogue/ItemDatabase.h>
#include <rogue/ItemEffect.h>
#include <rogue/ItemPrototype.h>
#include <rogue/ItemSpecialization.h>
#include <rogue/JSON.h>

namespace rogue {

static StatPoints parseStatPoints(const rapidjson::Value &V) {
  StatPoints P;
  P.Str = V["str"].GetInt();
  P.Dex = V["dex"].GetInt();
  P.Int = V["int"].GetInt();
  P.Vit = V["vit"].GetInt();
  return P;
}

static std::map<std::string, int> getItemIdsByName(const rapidjson::Value &V) {
  std::map<std::string, int> ItemIdsByName;

  int ItemId = 0;
  for (const auto &ItemProtoJson : V.GetArray()) {
    const auto Name = std::string(ItemProtoJson["name"].GetString());
    if (ItemIdsByName.count(Name) != 0) {
      throw std::runtime_error("Duplicate item name: " + Name);
    }
    ItemIdsByName.emplace(Name, ItemId++);
  }

  return ItemIdsByName;
}

template <typename T>
void parseReductionBuff(const rapidjson::Value &V, T &Buff) {
  auto TickAmount = V["reduce_amount"].GetDouble();
  auto TickPeriod = V["tick_period"].GetUint();
  auto RealDuration = TickPeriod * V["ticks"].GetUint();
  Buff.init(TickAmount, RealDuration, TickPeriod);
}

template <typename T>
void parseRegenerationBuff(const rapidjson::Value &V, T &Buff) {
  auto TickAmount = V["regen_amount"].GetDouble();
  auto TickPeriod = V["tick_period"].GetUint();
  auto RealDuration = TickPeriod * V["ticks"].GetUint();
  Buff.init(TickAmount, RealDuration, TickPeriod);
}

static std::shared_ptr<ItemEffect> createEffect(const ItemDatabase &DB,
                                                const rapidjson::Value &V) {
  static const std::map<std::string,
                        std::function<std::shared_ptr<ItemEffect>(
                            const ItemDatabase &, const rapidjson::Value &)>>
      Factories = {
          {"health_item_effect",
           [](const auto &, const auto &V) {
             const auto HealthValue = V["health_value"].GetDouble();
             return std::make_shared<HealItemEffect>(HealthValue);
           }},
          {"damage_item_effect",
           [](const auto &, const auto &V) {
             const auto DamageValue = V["damage_value"].GetDouble();
             return std::make_shared<DamageItemEffect>(DamageValue);
           }},
          {"poison_debuff_comp",
           [](const auto &, const auto &V) {
             PoisonDebuffComp Buff;
             parseReductionBuff(V, Buff);
             return makeApplyBuffItemEffect<PoisonDebuffComp, HealthComp>(Buff);
           }},
          {"health_regen_buff_comp",
           [](const auto &, const auto &V) {
             HealthRegenBuffComp Buff;
             parseRegenerationBuff(V, Buff);
             return makeApplyBuffItemEffect<HealthRegenBuffComp, HealthComp>(
                 Buff);
           }},
          {"bleeding_debuff_comp",
           [](const auto &, const auto &V) {
             BleedingDebuffComp Buff;
             parseReductionBuff(V, Buff);
             return makeApplyBuffItemEffect<BleedingDebuffComp, HealthComp>(
                 Buff);
           }},
          {"stats_buff_comp",
           [](const auto &, const auto &V) {
             StatPoints P = parseStatPoints(V["stats"]);
             StatsBuffComp Buff;
             Buff.Bonus = P;
             return makeApplyBuffItemEffect<StatsBuffComp, StatsComp>(Buff);
           }},
          {"armor_buff_comp",
           [](const auto &, const auto &V) {
             ArmorBuffComp Armor;
             Armor.PhysArmor = V["phys_armor"].GetDouble();
             Armor.MagicArmor = V["magic_armor"].GetDouble();
             return makeApplyBuffItemEffect<ArmorBuffComp, HealthComp>(Armor);
           }},
          {"block_comp",
           [](const auto &, const auto &V) {
             BlockBuffComp BC;
             BC.BlockChance = V["block_chance"].GetDouble();
             return makeApplyBuffItemEffect<BlockBuffComp, HealthComp>(BC);
           }},
          {"blinded_debuff_comp",
           [](const auto &, const auto &V) {
             BlindedDebuffComp BDC;
             BDC.TicksLeft = V["ticks"].GetUint();
             return makeApplyBuffItemEffect<BlindedDebuffComp>(BDC);
           }},
          {"melee_attack_comp",
           [](const auto &, const auto &V) {
             MeleeAttackComp MAC;
             MAC.PhysDamage = V["phys_damage"].GetDouble();
             MAC.MagicDamage = V["magic_damage"].GetDouble();
             MAC.APCost = V["ap_cost"].GetUint();
             return makeSetComponentEffect<MeleeAttackComp>(MAC);
           }},
          {"ranged_attack_comp",
           [](const auto &, const auto &V) {
             RangedAttackComp RAC;
             RAC.PhysDamage = V["phys_damage"].GetDouble();
             RAC.MagicDamage = V["magic_damage"].GetDouble();
             RAC.APCost = V["ap_cost"].GetUint();
             return makeSetComponentEffect<RangedAttackComp>(RAC);
           }},
          {"stats_buff_per_hit_comp",
           [](const auto &, const auto &V) {
             StatsBuffPerHitComp Buff;
             Buff.SBC.Bonus = parseStatPoints(V["stats"]);
             Buff.TickPeriod = V["ticks"].GetUint();
             Buff.TicksLeft = 0;
             Buff.TickPeriodsLeft = 0;
             Buff.MaxStacks = V["max_stacks"].GetUint();
             return makeApplyBuffItemEffect<StatsBuffPerHitComp, StatsComp>(
                 Buff);
           }},
          {"dismantle", [](const auto &DB, const auto &V) {
             std::vector<DismantleEffect::DismantleResult> Results;
             for (const auto &ItemJson : V["items"].GetArray()) {
               const auto ItemName = std::string(ItemJson["name"].GetString());
               const auto ItemId = DB.getItemId(ItemName);
               const auto Amount = ItemJson["amount"].GetUint();
               Results.push_back({ItemId, Amount});
             }
             return std::make_shared<DismantleEffect>(DB, std::move(Results));
           }}};

  const auto EffectType = V["type"].GetString();
  const auto It = Factories.find(EffectType);
  if (It == Factories.end()) {
    throw std::out_of_range("Unknown item effect: " + std::string(EffectType));
  }

  return It->second(DB, V);
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

static LootTable::LootSlot createLootItemSlot(const ItemDatabase &DB,
                                              const rapidjson::Value &V) {
  const auto Weight = V["weight"].GetInt();
  const auto ItemName = std::string(V["name"].GetString());
  const auto ItId = DB.getItemId(ItemName);
  const auto MinCount = V["min_count"].GetUint();
  const auto MaxCount = V["max_count"].GetUint();
  return LootTable::LootSlot{
      std::make_shared<LootItem>(ItId, MinCount, MaxCount), Weight};
}

static LootTable::LootSlot createLootTableSlot(const ItemDatabase &DB,
                                               const rapidjson::Value &V) {
  const auto Weight = V["weight"].GetInt();
  const auto LootTbName = V["ref"].GetString();
  return LootTable::LootSlot{DB.getLootTable(LootTbName), Weight};
}

static LootTable::LootSlot createLootNullSlot(const rapidjson::Value &V) {
  const auto Weight = V["weight"].GetInt();
  return LootTable::LootSlot{nullptr, Weight};
}

static void fillLootTable(const ItemDatabase &DB, const rapidjson::Value &V,
                          LootTable &LootTb) {
  const auto NumRolls = V["rolls"].GetUint();
  std::vector<LootTable::LootSlot> Slots;
  for (const auto &SlotJson : V["slots"].GetArray()) {
    const auto Type = std::string(SlotJson["type"].GetString());
    if (Type == "item") {
      Slots.push_back(createLootItemSlot(DB, SlotJson));
    } else if (Type == "table") {
      Slots.push_back(createLootTableSlot(DB, SlotJson));
    } else if (Type == "null") {
      Slots.push_back(createLootNullSlot(SlotJson));
    } else {
      throw std::out_of_range("Invalid loot table slot type: " + Type);
    }
  }

  LootTb.reset(NumRolls, Slots);
}

ItemDatabase ItemDatabase::load(const std::filesystem::path &ItemDbConfig) {
  ItemDatabase DB;

  const auto SchemaPath = ItemDbConfig.parent_path() / "item_db_schema.json";
  auto [DocStr, Doc] = loadJSON(ItemDbConfig, &SchemaPath);

  // Get map of item Ids and verify unique names
  DB.ItemIdsByName = getItemIdsByName(Doc["item_prototypes"]);

  // Create effects
  std::map<std::string, std::shared_ptr<ItemEffect>> Effects;
  const auto &EffectsJson = Doc["item_effects"].GetObject();
  for (const auto &[K, V] : EffectsJson) {
    const auto EffectName = std::string(K.GetString());
    // Check if already registered
    if (Effects.count(EffectName) != 0) {
      throw std::runtime_error("Duplicate item effect: " + EffectName);
    }
    auto Effect = createEffect(DB, V);
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

    std::vector<EffectInfo> EffectInfos;
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

  // Create loot tables
  const auto &LootTablesJson = Doc["loot_tables"].GetObject();
  for (const auto &[K, V] : LootTablesJson) {
    const auto Name = std::string(K.GetString());
    // Check if already registered
    DB.addLootTable(Name);
  }
  for (const auto &[K, V] : LootTablesJson) {
    const auto Name = std::string(K.GetString());
    auto &LootTb = DB.getLootTable(Name);
    fillLootTable(DB, V, *LootTb);
  }

  return DB;
}

int ItemDatabase::getItemId(const std::string &ItemName) const {
  const auto It = ItemIdsByName.find(ItemName);
  if (It == ItemIdsByName.end()) {
    throw std::out_of_range("Unknown item name: " + ItemName);
  }
  return It->second;
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
  if (ItemProtos.empty()) {
    throw std::out_of_range("No item prototypes");
  }
  const auto Idx = rand() % ItemProtos.size();
  const auto It = std::next(ItemProtos.begin(), Idx);
  return It->first;
}

LootTable &ItemDatabase::addLootTable(const std::string &Name) {
  auto [It, Inserted] = LootTables.emplace(Name, std::make_shared<LootTable>());
  if (!Inserted) {
    throw std::out_of_range("Loot table with name '" + Name +
                            "' already exists");
  }
  return *It->second;
}

const std::shared_ptr<LootTable> &
ItemDatabase::getLootTable(const std::string &Name) const {
  const auto It = LootTables.find(Name);
  if (It == LootTables.end()) {
    throw std::out_of_range("Unknown loot table: " + Name);
  }
  return It->second;
}

} // namespace rogue