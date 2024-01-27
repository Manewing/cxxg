#include <fstream>
#include <ranges>
#include <rogue/Components/Buffs.h>
#include <rogue/Components/Combat.h>
#include <rogue/CraftingHandler.h>
#include <rogue/ItemDatabase.h>
#include <rogue/ItemEffect.h>
#include <rogue/ItemEffectImpl.h>
#include <rogue/ItemPrototype.h>
#include <rogue/ItemSpecialization.h>
#include <rogue/JSON.h>
#include <rogue/JSONHelpers.h>

namespace rogue {

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

CoHTargetPoisonDebuffComp
parseCoHTargetPoisonDebuffComp(const rapidjson::Value &V) {
  CoHTargetPoisonDebuffComp Effect;
  Effect.Chance = V["chance"].GetDouble();
  parseReductionBuff(V["buff"], Effect.Buff);
  return Effect;
}

CoHTargetBleedingDebuffComp
parseCoHTargetBleedingDebuffComp(const rapidjson::Value &V) {
  CoHTargetBleedingDebuffComp Effect;
  Effect.Chance = V["chance"].GetDouble();
  parseReductionBuff(V["buff"], Effect.Buff);
  return Effect;
}

CoHTargetBlindedDebuffComp
parseCoHTargetBlindedDebuffComp(const rapidjson::Value &V) {
  CoHTargetBlindedDebuffComp Effect;
  Effect.Chance = V["chance"].GetDouble();
  Effect.Buff.TicksLeft = V["buff"]["ticks"].GetUint();
  return Effect;
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
          {"mana_item_effect",
           [](const auto &, const auto &V) {
             const auto ManaValue = V["mana_value"].GetDouble();
             return std::make_shared<ManaItemEffect>(ManaValue);
           }},
          {"damage_item_effect",
           [](const auto &, const auto &V) {
             const auto DamageValue = V["damage_value"].GetDouble();
             return std::make_shared<DamageItemEffect>(DamageValue);
           }},
          {"poison_debuff_comp",
           [](const auto &, const auto &V) {
             PoisonDebuffComp Buff;
             parseReductionBuff(V["buff"], Buff);
             return std::make_shared<PoisonDebuffEffect>(Buff);
           }},
          {"health_regen_buff_comp",
           [](const auto &, const auto &V) {
             HealthRegenBuffComp Buff;
             parseRegenerationBuff(V["buff"], Buff);
             return std::make_shared<HealthRegenBuffEffect>(Buff);
           }},
          {"mana_regen_buff_comp",
           [](const auto &, const auto &V) {
             ManaRegenBuffComp Buff;
             parseRegenerationBuff(V["buff"], Buff);
             return std::make_shared<ManaRegenBuffEffect>(Buff);
           }},
          {"bleeding_debuff_comp",
           [](const auto &, const auto &V) {
             BleedingDebuffComp Buff;
             parseReductionBuff(V["buff"], Buff);
             return std::make_shared<BleedingDebuffEffect>(Buff);
           }},
          {"stats_buff_comp",
           [](const auto &, const auto &V) {
             StatPoints P = parseStatPoints(V["stats"]);
             StatsBuffComp Buff;
             Buff.Bonus = P;
             return std::make_shared<StatsBuffEffect>(Buff);
           }},
          {"stats_timed_buff_comp",
           [](const auto &, const auto &V) {
             StatPoints P = parseStatPoints(V["stats"]);
             StatsTimedBuffComp Buff;
             Buff.Bonus = P;
             Buff.TicksLeft = V["ticks"].GetUint();
             return std::make_shared<StatsTimedBuffEffect>(Buff);
           }},
          {"armor_buff_comp",
           [](const auto &, const auto &V) {
             ArmorBuffComp Armor;
             Armor.PhysArmor = V["phys_armor"].GetDouble();
             Armor.MagicArmor = V["magic_armor"].GetDouble();
             return std::make_shared<ArmorBuffEffect>(Armor);
           }},
          {"block_comp",
           [](const auto &, const auto &V) {
             BlockBuffComp BC;
             BC.BlockChance = V["block_chance"].GetDouble();
             return std::make_shared<BlockBuffEffect>(BC);
           }},
          {"blinded_debuff_comp",
           [](const auto &, const auto &V) {
             BlindedDebuffComp BDC;
             BDC.TicksLeft = V["buff"]["ticks"].GetUint();
             return std::make_shared<BlindedDebuffEffect>(BDC);
           }},
          {"mind_vision_buff_comp",
           [](const auto &, const auto &V) {
             MindVisionBuffComp MVBC;
             MVBC.TicksLeft = V["ticks"].GetUint();
             MVBC.Range = V["range"].GetUint();
             return std::make_shared<MindVisionBuffEffect>(MVBC);
           }},
          {"invisibility_buff_comp",
           [](const auto &, const auto &V) {
             InvisibilityBuffComp IBC;
             IBC.TicksLeft = V["ticks"].GetUint();
             return std::make_shared<InvisibilityBuffEffect>(IBC);
           }},
          {"melee_attack_comp",
           [](const auto &, const auto &V) {
             MeleeAttackComp MAC;
             MAC.PhysDamage = V["phys_damage"].GetDouble();
             MAC.MagicDamage = V["magic_damage"].GetDouble();
             MAC.APCost = V["ap_cost"].GetUint();
             if (V.HasMember("mana_cost")) {
               MAC.ManaCost = V["mana_cost"].GetUint();
             }
             return std::make_shared<SetMeleeCompEffect>(MAC);
           }},
          {"ranged_attack_comp",
           [](const auto &, const auto &V) {
             RangedAttackComp RAC;
             RAC.PhysDamage = V["phys_damage"].GetDouble();
             RAC.MagicDamage = V["magic_damage"].GetDouble();
             RAC.APCost = V["ap_cost"].GetUint();
             if (V.HasMember("mana_cost")) {
               RAC.ManaCost = V["mana_cost"].GetUint();
             }
             return std::make_shared<SetRangedCompEffect>(RAC);
           }},
          {"stats_buff_per_hit_comp",
           [](const auto &, const auto &V) {
             StatsBuffPerHitComp Buff;
             Buff.SBC.Bonus = parseStatPoints(V["stats"]);
             Buff.TickPeriod = V["ticks"].GetUint();
             Buff.TicksLeft = 0;
             Buff.TickPeriodsLeft = 0;
             Buff.MaxStacks = V["max_stacks"].GetUint();
             return std::make_shared<StatsBuffPerHitEffect>(Buff);
           }},
          {"chance_on_hit_to_apply_poison",
           [](const auto &, const auto &V) {
             auto Buff = parseCoHTargetPoisonDebuffComp(V);
             return std::make_shared<CoHTargetPoisonDebuffEffect>(Buff);
           }},
          {"chance_on_hit_to_apply_bleeding",
           [](const auto &, const auto &V) {
             auto Buff = parseCoHTargetBleedingDebuffComp(V);
             return std::make_shared<CoHTargetBleedingDebuffEffect>(Buff);
           }},
          {"chance_on_hit_to_apply_blinded",
           [](const auto &, const auto &V) {
             auto Buff = parseCoHTargetBlindedDebuffComp(V);
             return std::make_shared<CoHTargetBlindedDebuffEffect>(Buff);
           }},
          {"life_steal",
           [](const auto &, const auto &V) {
             LifeStealBuffComp LSBC;
             LSBC.Percent = V["percent"].GetDouble();
             LSBC.BonusHP = V["bonus_hp"].GetDouble();
             return std::make_shared<LifeStealBuffEffect>(LSBC);
           }},
           {"spawn_entity_effect",
           [](const auto &, const auto &V) {
            auto EntityName = std::string(V["entity_name"].GetString());
            double Chance = V["chance"].GetDouble();
            return std::make_shared<SpawnEntityEffect>(EntityName, Chance);
           }
           },
          {"disc_area_hit_effect",
           [](const auto &, const auto &V) {
             auto Name = std::string(V["name"].GetString());
             auto Radius = V["radius"].GetUint();
             auto PhysDamage = V["phys_damage"].GetDouble();
             auto MagicDamage = V["magic_damage"].GetDouble();
             std::optional<CoHTargetBleedingDebuffComp> Bleeding;
             if (V.HasMember("bleeding")) {
               Bleeding = parseCoHTargetBleedingDebuffComp(V["bleeding"]);
             }
             std::optional<CoHTargetPoisonDebuffComp> Poison;
             if (V.HasMember("poison")) {
               Poison = parseCoHTargetPoisonDebuffComp(V["poison"]);
             }
             std::optional<CoHTargetBlindedDebuffComp> Blinded;
             if (V.HasMember("blinded")) {
               Blinded = parseCoHTargetBlindedDebuffComp(V["blinded"]);
             }
             unsigned MinTicks = 1, MaxTicks = 1;
             if (V.HasMember("min_ticks")) {
               MinTicks = V["min_ticks"].GetUint();
             }
             if (V.HasMember("max_ticks")) {
               MaxTicks = V["max_ticks"].GetUint();
             }
             bool CanHurtSource = true;
             if (V.HasMember("can_hurt_source")) {
               CanHurtSource = V["can_hurt_source"].GetBool();
             }
             bool CanHurtFaction = true;
              if (V.HasMember("can_hurt_faction")) {
                CanHurtFaction = V["can_hurt_faction"].GetBool();
              }
             auto DecreasePercent = V["decrease_percent"].GetDouble();
             auto T = parseTile(V["effect_tile"]);
             return std::make_shared<DiscAreaHitEffect>(
                 Name, Radius, PhysDamage, MagicDamage, Bleeding, Poison,
                 Blinded, T, DecreasePercent, MinTicks, MaxTicks,
                 CanHurtSource, CanHurtFaction);
           }},
          {"smite_effect",
           [](const auto &, const auto &V) {
             auto Name = std::string(V["name"].GetString());
             auto DamagePercent = V["damage_percent"].GetDouble();
             return std::make_shared<SmiteEffect>(Name, DamagePercent);
           }},
          {"sweeping_strike_effect",
           [](const auto &, const auto &V) {
             auto Name = std::string(V["name"].GetString());
             auto DamagePercent = V["damage_percent"].GetDouble();
             auto T = parseTile(V["effect_tile"]);
             return std::make_shared<SweepingStrikeEffect>(Name, DamagePercent,
                                                           T);
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

  const auto EffectType = std::string(V["type"].GetString());
  const auto It = Factories.find(EffectType);
  if (It == Factories.end()) {
    throw std::out_of_range("ItemDatabase: Unknown item effect: " +
                            std::string(EffectType));
  }

  return It->second(DB, V);
}

static void addDefaultConstructEffect(
    std::map<std::string, std::shared_ptr<ItemEffect>> &Effects,
    const std::string &EffectName, const std::shared_ptr<ItemEffect> &Effect) {
  const auto It = Effects.find(EffectName);
  if (It != Effects.end()) {
    throw std::out_of_range(
        "ItemDatabase: Conflicting special effect (reserved name): " +
        EffectName);
  }
  Effects[EffectName] = Effect;
}

static void addDefaultConstructEffects(
    std::map<std::string, std::shared_ptr<ItemEffect>> &Effects) {
  addDefaultConstructEffect(Effects, "null", std::make_shared<NullEffect>());
  addDefaultConstructEffect(Effects, "remove_poison_effect",
                            std::make_shared<RemovePoisonEffect>());
  addDefaultConstructEffect(Effects, "remove_poison_debuff",
                            std::make_shared<RemovePoisonDebuffEffect>());
  addDefaultConstructEffect(Effects, "remove_bleeding_effect",
                            std::make_shared<RemoveBleedingEffect>());
  addDefaultConstructEffect(Effects, "remove_bleeding_debuff",
                            std::make_shared<RemoveBleedingDebuffEffect>());
  addDefaultConstructEffect(Effects, "remove_blinded_debuff",
                            std::make_shared<RemoveBlindedDebuffEffect>());
  addDefaultConstructEffect(Effects, "remove_health_regen_effect",
                            std::make_shared<RemoveHealthRegenEffect>());
  addDefaultConstructEffect(Effects, "remove_health_regen_buff",
                            std::make_shared<RemoveHealthRegenBuffEffect>());
  addDefaultConstructEffect(Effects, "remove_mana_regen_effect",
                            std::make_shared<RemoveManaRegenEffect>());
  addDefaultConstructEffect(Effects, "remove_mana_regen_buff",
                            std::make_shared<RemoveManaRegenBuffEffect>());
  addDefaultConstructEffect(Effects, "learn_crafting_recipe",
                            std::make_shared<LearnRecipeEffect>());
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

static EffectAttributes createEffectAttrs(const rapidjson::Value &V) {
  EffectAttributes Attrs;
  Attrs.Flags = CapabilityFlags::fromString(V["type"].GetString());
  if (V.HasMember("ap_cost")) {
    Attrs.APCost = V["ap_cost"].GetDouble();
  }
  if (V.HasMember("mana_cost")) {
    Attrs.ManaCost = V["mana_cost"].GetDouble();
  }
  if (V.HasMember("health_cost")) {
    Attrs.HealthCost = V["health_cost"].GetDouble();
  }
  return Attrs;
}

static void fillLootTable(const ItemDatabase &DB, const rapidjson::Value &V,
                          LootTable &LootTb) {
  const auto NumRolls = V["rolls"].GetUint();
  bool PickAndReturn = false;
  if (V.HasMember("pick_and_return")) {
    PickAndReturn = V["pick_and_return"].GetBool();
  }
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

  LootTb.reset(NumRolls, Slots, PickAndReturn);
}

ItemDatabase ItemDatabase::load(const std::filesystem::path &ItemDbConfig,
                                const std::filesystem::path *SchemaPath) {
  ItemDatabase DB;

  const auto SchemaPathDefault =
      ItemDbConfig.parent_path() / "schemas" / "item_db_schema.json";
  const auto *const SchemaPathPtr =
      SchemaPath ? SchemaPath : &SchemaPathDefault;
  auto [DocStr, Doc] = loadJSON(ItemDbConfig, SchemaPathPtr);

  // Get map of item Ids and verify unique names
  DB.ItemIdsByName = getItemIdsByName(Doc["item_prototypes"]);

  // Create effects
  const auto &EffectsJson = Doc["item_effects"].GetObject();
  for (const auto &[K, V] : EffectsJson) {
    const auto EffectName = std::string(K.GetString());
    // Check if already registered
    if (DB.Effects.count(EffectName) != 0) {
      throw std::runtime_error("Duplicate item effect: " + EffectName);
    }
    auto Effect = createEffect(DB, V);
    DB.Effects.emplace(EffectName, Effect);
  }
  addDefaultConstructEffects(DB.Effects);

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

  // Create loot tables
  const auto &LootTablesJson = Doc["loot_tables"].GetObject();
  for (const auto &[K, V] : LootTablesJson) {
    const auto Name = std::string(K.GetString());
    // Check if already registered
    DB.addLootTable(Name);
  }

  // Create item prototypes
  const auto &ItemProtosJson = Doc["item_prototypes"].GetArray();
  for (const auto &ItemProtoJson : ItemProtosJson) {
    const auto Name = std::string(ItemProtoJson["name"].GetString());
    const auto Description = ItemProtoJson["description"].GetString();
    const auto MaxStackSize = ItemProtoJson["max_stack_size"].GetInt();

    ItemType ItType = ItemType::None;
    for (const auto &ItemType : ItemProtoJson["types"].GetArray()) {
      const auto ItemTypeStr = ItemType.GetString();
      ItType |= ItemType::fromString(ItemTypeStr);
    }

    std::vector<EffectInfo> EffectInfos;
    for (const auto &CapJson : ItemProtoJson["capabilities"].GetArray()) {
      const auto &CapInfo = CapJson.GetObject();
      const auto Attrs = createEffectAttrs(CapJson);
      const auto EffectName = std::string(CapInfo["effect"].GetString());
      EffectInfos.push_back({Attrs, DB.getItemEffect(EffectName)});
    }

    std::unique_ptr<ItemSpecializations> Specialization;
    if (ItemProtoJson.HasMember("specializations")) {
      Specialization = std::make_unique<ItemSpecializations>();
      for (const auto &SpecJson : ItemProtoJson["specializations"].GetArray()) {
        const auto &SpecInfo = SpecJson.GetObject();
        const auto Attrs = createEffectAttrs(SpecJson);
        const auto Spec =
            Specializations.at(SpecInfo["specialization"].GetString());
        Specialization->addSpecialization(Attrs, Spec);
      }
    }

    std::shared_ptr<LootTable> Enhancements = nullptr;
    if (ItemProtoJson.HasMember("enhancements")) {
      const auto &LootTbName = ItemProtoJson["enhancements"].GetString();
      const auto &LootTb = DB.getLootTable(LootTbName);
      Enhancements = LootTb;
    }

    ItemPrototype Proto(DB.getNewItemId(), Name, Description, ItType,
                        MaxStackSize, std::move(EffectInfos));
    DB.addItemProto(Proto, Specialization.get(), Enhancements);
  }

  // Fill loot tables
  for (const auto &[K, V] : LootTablesJson) {
    const auto Name = std::string(K.GetString());
    auto &LootTb = DB.getLootTable(Name);
    fillLootTable(DB, V, *LootTb);
  }

  return DB;
}

int ItemDatabase::getNewItemId() { return MaxItemId++; }

int ItemDatabase::getItemId(const std::string &ItemName) const {
  const auto It = ItemIdsByName.find(ItemName);
  if (It == ItemIdsByName.end()) {
    throw std::out_of_range("Unknown item name: " + ItemName);
  }
  return It->second;
}

const std::map<int, ItemPrototype> &ItemDatabase::getItemProtos() const {
  return ItemProtos;
}

const ItemPrototype &ItemDatabase::getItemProto(int ItemId) const {
  const auto It = ItemProtos.find(ItemId);
  if (It == ItemProtos.end()) {
    throw std::out_of_range("Unknown item id: " + std::to_string(ItemId));
  }
  return It->second;
}

const ItemSpecializations *ItemDatabase::getItemSpec(int ItemId) const {
  const auto It = ItemSpecs.find(ItemId);
  if (It == ItemSpecs.end()) {
    return nullptr;
  }
  return &It->second;
}

void ItemDatabase::addItemProto(
    const ItemPrototype &ItemProto, const ItemSpecializations *ItemSpec,
    const std::shared_ptr<LootTable> &Enhancements) {
  // FIXME ID not yet taken
  assert(ItemProtos.count(ItemProto.ItemId) == 0);
  ItemProtos.emplace(ItemProto.ItemId, ItemProto);
  if (ItemSpec) {
    ItemSpecs.emplace(ItemProto.ItemId, *ItemSpec);
  }
  if (Enhancements) {
    ItemEnhancements.emplace(ItemProto.ItemId, Enhancements);
  }
}

Item ItemDatabase::createItem(int ItemId, int StackSize,
                              bool AllowEnchanting) const {
  auto It = ItemProtos.find(ItemId);
  if (It == ItemProtos.end()) {
    throw std::out_of_range("Unknown item id: " + std::to_string(ItemId));
  }
  // Actualize the specialization
  std::shared_ptr<ItemPrototype> Spec = nullptr;
  if (auto SpecIt = ItemSpecs.find(ItemId); SpecIt != ItemSpecs.end()) {
    Spec = SpecIt->second.actualize(It->second);
  }

  auto NewItem = Item(It->second, StackSize, Spec);

  // Craft enhancements
  if (auto EnhIt = ItemEnhancements.find(ItemId);
      AllowEnchanting && EnhIt != ItemEnhancements.end() && EnhIt->second) {
    auto LootRewards = EnhIt->second->generateLoot();
    std::vector<Item> LootItems;
    for (const auto &Reward : LootRewards) {
      for (unsigned I = 0; I < Reward.Count; ++I) {
        LootItems.push_back(createItem(Reward.ItId, 1));
      }
    }
    CraftingHandler Crafter;
    while (!LootItems.empty()) {
      auto NextEnhancement = LootItems.back();
      LootItems.pop_back();

      // Try to craft, if it does not succeed abort (invalid combination)
      auto Result = Crafter.tryCraft({NewItem, NextEnhancement});
      if (!Result || Result->size() != 1) {
        throw std::runtime_error("Invalid crafting result for " +
                                 NewItem.getName() + " and " +
                                 NextEnhancement.getName());
      }

      NewItem = Result->at(0);
    }
  }

  return NewItem;
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

const std::map<std::string, std::shared_ptr<LootTable>> &
ItemDatabase::getLootTables() const {
  return LootTables;
}

const std::shared_ptr<ItemEffect> &
ItemDatabase::getItemEffect(const std::string &Name) const {
  const auto It = Effects.find(Name);
  if (It == Effects.end()) {
    throw std::out_of_range("Unknown item effect: " + Name);
  }
  return It->second;
}

} // namespace rogue