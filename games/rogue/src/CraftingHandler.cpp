#include <rogue/ItemDatabase.h>
#include <rogue/ItemEffect.h>
#include <rogue/CraftingHandler.h>

namespace rogue {

CraftingHandler::CraftingHandler(ItemDatabase &ItemDb) : ItemDb(ItemDb) {}

std::optional<Item> CraftingHandler::tryCraft(const std::vector<Item> &Items) {
  if (Items.size() < 2) {
    return std::nullopt;
  }
  auto &First = Items.at(0);
  auto &Second = Items.at(1);

  // If both items are crafting items this indicates a crafting recipe,
  // otherwise it's a modification of an item or invalid combination
  if ((First.getType() & ItemType::Crafting) != ItemType::None &&
      (Second.getType() & ItemType::Crafting) != ItemType::None) {
    // TODO: craft item if it matches recipe,
  }

  // Filter out any invalid combinations
  const auto IsValid =
      ((First.getType() & ItemType::CraftingBase) != ItemType::None &&
       (Second.getType() & ItemType::Crafting) != ItemType::None) ||
      ((First.getType() & ItemType::EquipmentMask) != ItemType::None &&
       (Second.getType() & ItemType::Crafting) != ItemType::None);
  if (!IsValid) {
    return std::nullopt;
  }

  return craftEnhancedItem(Items);
}

Item CraftingHandler::craftEnhancedItem(const std::vector<Item> &Items) {
  auto &First = Items.at(0);
  auto Flags = First.getCapabilityFlags();

  // Create new item prototype, we explicitly copy all effects from the first
  // item (including the specialization effects). The item specialization will
  // not be copied (the first item is already specialized).
  auto NewItemId = ItemDb.getNewItemId();
  ItemPrototype Proto(NewItemId, First.getName(), First.getDescription(),
                      First.getType(), First.getMaxStackSize(),
                      First.getAllEffects());

  // Combine effects from all other items
  for (std::size_t Idx = 1; Idx < Items.size(); ++Idx) {
    auto &Item = Items.at(Idx);
    for (const auto &Info : Item.getAllEffects()) {
      if ((Info.Flags & Flags) != CapabilityFlags::None) {
        Proto.Effects.push_back(Info);
      }
    }
  }

  // Create new effects from effects in prototype that can be added
  std::vector<EffectInfo> NewEffects;
  std::optional<EffectInfo> NullInfo;
  CapabilityFlags EffectFlags = CapabilityFlags::None;
  for (const auto &Info : Proto.Effects) {
    auto Effect = Info.Effect.get();

    // Handle NullEffect separately, we only keep the effect if no other
    // effect has the same flags
    if (dynamic_cast<NullEffect *>(Info.Effect.get())) {
      if (NullInfo) {
        NullInfo->Flags |= Info.Flags;
      } else {
        NullInfo = Info;
      }
      continue;
    }

    // Handle RemoveEffectBase separately, we remove all newly added effects
    // that are removed by this effect
    if (auto *RM = dynamic_cast<const RemoveEffectBase *>(Effect)) {
      auto RMFlags = Info.Flags;
      auto It = std::remove_if(NewEffects.begin(), NewEffects.end(),
                               [RM, RMFlags](const EffectInfo &Info) {
                                 if (Info.Flags != RMFlags) {
                                   return false;
                                 }
                                 return RM->removesEffect(*Info.Effect);
                               });
      NewEffects.erase(It, NewEffects.end());
      continue;
    }

    EffectFlags |= Info.Flags;

    for (auto &NewInfo : NewEffects) {
      if (NewInfo.Flags != Info.Flags) {
        continue;
      }
      auto NewEffect = NewInfo.Effect.get();
      if (NewEffect->canAddFrom(*Effect)) {
        NewEffect->addFrom(*Effect);
        Effect = nullptr;
        break;
      }
    }
    if (Effect) {
      NewEffects.push_back({Info.Flags, Effect->clone()});
    }
  }
  if (NullInfo && (NullInfo->Flags & ~EffectFlags) != CapabilityFlags::None) {
    NewEffects.push_back(*NullInfo);
  }
  Proto.Effects = NewEffects;

  // Register the new item prototype
  ItemDb.addItemProto(std::move(Proto));

  // Create the new item
  return ItemDb.createItem(NewItemId, /*StackSize=*/1);
}

} // namespace rogue