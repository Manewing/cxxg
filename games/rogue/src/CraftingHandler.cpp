#include <rogue/CraftingHandler.h>
#include <rogue/ItemDatabase.h>
#include <rogue/ItemEffect.h>

#include <iostream>
namespace rogue {

const std::optional<std::vector<CraftingNode::ItemId>> &
CraftingNode::search(const std::vector<Item> &Items) const {
  static const std::optional<std::vector<ItemId>> Empty;
  if (Items.empty()) {
    return Empty;
  }

  const CraftingNode *Node = this;
  for (auto &It : Items) {
    auto ItNode = Node->Children.find(It.getId());
    if (ItNode == Node->Children.end()) {
      return Empty;
    }
    Node = &ItNode->second;
  }

  return Node->ResultItems;
}

CraftingHandler::CraftingHandler(const ItemDatabase &ItemDb) : ItemDb(ItemDb) {}

void CraftingHandler::addRecipe(const CraftingRecipe &Recipe) {
  if (Recipe.getResultItems().empty() || Recipe.getRequiredItems().empty()) {
    throw std::runtime_error("CraftingHandler::addRecipe: Empty recipe");
  }

  CraftingNode *Node = &Tree;
  for (auto &Id : Recipe.getRequiredItems()) {
    Node = &Node->Children[Id];
  }

  if (Node->ResultItems) {
    throw std::runtime_error("CraftingHandler::addRecipe: Duplicate recipe");
  }
  Node->ResultItems = Recipe.getResultItems();
}

std::optional<std::vector<Item>>
CraftingHandler::tryCraft(const std::vector<Item> &Items) {
  if (Items.size() < 2) {
    return std::nullopt;
  }
  auto &First = Items.at(0);
  auto &Second = Items.at(1);

  // If both items are crafting items this indicates a crafting recipe,
  // otherwise it's a modification of an item or invalid combination
  if ((First.getType() & (ItemType::Crafting | ItemType::CraftingBase)) &&
      (Second.getType() & ItemType::Crafting)) {
    auto Result = Tree.search(Items);
    if (Result) {
      std::vector<Item> CraftedItems;
      for (auto &Id : *Result) {
        CraftedItems.push_back(ItemDb.createItem(Id));
      }
      return CraftedItems;
    }
  }

  // Filter out any invalid combinations
  const auto IsValid = ((First.getType() & ItemType::CraftingBase) &&
                        (Second.getType() & ItemType::Crafting)) ||
                       ((First.getType() & ItemType::EquipmentMask) &&
                        (Second.getType() & ItemType::Crafting));
  if (!IsValid) {
    return std::nullopt;
  }

  return std::vector<Item>{craftEnhancedItem(Items)};
}

namespace {

std::vector<EffectInfo> getAllEffects(const std::vector<Item> &Items,
                                      CapabilityFlags Flags) {
  std::vector<EffectInfo> AllEffects = Items.front().getAllEffects();
  for (std::size_t Idx = 1; Idx < Items.size(); ++Idx) {
    auto &It = Items.at(Idx);
    for (const auto &Info : It.getAllEffects()) {
      if (Info.Flags & Flags) {
        AllEffects.push_back(Info);
      }
    }
  }
  return AllEffects;
}

/// Computes new effects by combining all effects in the order they are provided
std::vector<EffectInfo>
computeNewEffects(const std::vector<EffectInfo> &AllEffects) {
  std::vector<EffectInfo> NewEffects;
  std::optional<EffectInfo> NullInfo;
  CapabilityFlags EffectFlags = CapabilityFlags::None;
  for (const auto &Info : AllEffects) {
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
  if (NullInfo && (NullInfo->Flags & ~EffectFlags)) {
    NewEffects.push_back(*NullInfo);
  }
  return NewEffects;
}

} // namespace

Item CraftingHandler::craftEnhancedItem(const std::vector<Item> &Items) {
  auto &First = Items.at(0);
  auto Flags = First.getCapabilityFlags();

  // Create new item prototype to use as the overriding item specialization, we
  // explicitly copy all effects from the first item (including the
  // specialization effects). The item specialization will not be copied (the
  // first item is already specialized).
  auto Special = std::make_shared<ItemPrototype>(
      -1, First.getName(), First.getDescription(), First.getType(),
      First.getMaxStackSize(), std::vector<EffectInfo>{});

  // Combine effects from all other items
  Special->Effects = getAllEffects(Items, Flags);
  Special->Effects = computeNewEffects(Special->Effects);

  // Create the new item
  return Item(First.getProto(), /*StackSize=*/1, Special,
              /*SpecOverrides=*/true);
}

} // namespace rogue