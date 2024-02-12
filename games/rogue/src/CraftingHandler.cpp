#include <rogue/CraftingHandler.h>
#include <rogue/ItemDatabase.h>
#include <rogue/ItemEffect.h>

namespace rogue {

static const std::optional<CraftingNode::CraftingResult> EmptyResult;

const std::optional<CraftingNode::CraftingResult> &
CraftingNode::search(const std::vector<Item> &Items) const {
  if (Items.empty()) {
    return EmptyResult;
  }

  const CraftingNode *Node = this;
  for (auto &It : Items) {
    auto ItNode = Node->Children.find(It.getId());
    if (ItNode == Node->Children.end()) {
      return EmptyResult;
    }
    Node = &ItNode->second;
  }

  return Node->Result;
}

CraftingHandler::CraftingHandler(const ItemDatabase &ItemDb)
    : ItemDb(&ItemDb) {}

void CraftingHandler::addRecipe(CraftingRecipeId RecipeId,
                                const CraftingRecipe &Recipe) {
  if (Recipe.getResultItems().empty() || Recipe.getRequiredItems().empty()) {
    throw std::runtime_error("CraftingHandler::addRecipe: Empty recipe");
  }

  CraftingNode *Node = &Tree;
  for (auto &Id : Recipe.getRequiredItems()) {
    Node = &Node->Children[Id];
  }

  if (Node->Result) {
    throw std::runtime_error("CraftingHandler::addRecipe: Duplicate recipe");
  }
  Node->Result =
      CraftingNode::CraftingResult{RecipeId, Recipe.getResultItems()};
}

const ItemDatabase *CraftingHandler::getItemDb() const { return ItemDb; }

const ItemDatabase &CraftingHandler::getItemDbOrFail() const {
  if (!ItemDb) {
    throw std::runtime_error("CraftingHandler::getItemDbOrFail: No ItemDb");
  }
  return *ItemDb;
}

const std::optional<CraftingNode::CraftingResult> &
CraftingHandler::getCraftingRecipeResultOrNone(
    const std::vector<Item> &Items) const {
  if (Items.size() < 2) {
    return EmptyResult;
  }
  auto &First = Items.at(0);
  auto &Second = Items.at(1);

  // If both items are crafting items this indicates a crafting recipe,
  // otherwise it's a modification of an item or invalid combination
  if (!(First.getType() & (ItemType::Crafting | ItemType::CraftingBase)) ||
      !(Second.getType() & ItemType::Crafting)) {
    return EmptyResult;
  }

  return Tree.search(Items);
}

std::optional<std::vector<Item>>
CraftingHandler::tryCraft(const std::vector<Item> &Items) const {
  if (Items.size() < 2) {
    return std::nullopt;
  }
  auto &First = Items.at(0);
  auto &Second = Items.at(1);

  // If both items are crafting items this indicates a crafting recipe,
  // otherwise it's a modification of an item or invalid combination
  if (auto CraftedOrNone = tryCraftAsRecipe(Items)) {
    return CraftedOrNone;
  }

  // Filter out any invalid combinations
  const bool IsValid = ((First.getType() & ItemType::CraftingBase) &&
                        (Second.getType() & ItemType::Crafting)) ||
                       ((First.getType() & ItemType::EquipmentMask) &&
                        (Second.getType() & ItemType::Crafting));
  if (!IsValid) {
    return std::nullopt;
  }

  const bool IsValidEnhancement =
      std::all_of(Items.begin() + 1, Items.end(), [&First](const auto &Other) {
        return First.getType() &
               Other.getEnhanceFilterType().value_or(ItemType::AnyMask);
      });
  if (!IsValidEnhancement) {
    return std::nullopt;
  }

  return std::vector<Item>{craftEnhancedItem(Items)};
}

std::optional<std::vector<Item>>
CraftingHandler::tryCraftAsRecipe(const std::vector<Item> &Items) const {
  auto Result = getCraftingRecipeResultOrNone(Items);
  if (!Result) {
    return std::nullopt;
  }

  std::vector<Item> CraftedItems;
  for (auto &Id : Result->Items) {
    CraftedItems.push_back(
        ItemDb->createItem(Id, /*StackSize=*/1, /*AllowEnchanting=*/false));
  }
  return CraftedItems;
}

namespace {

/// Return all effects with an non-empty subset of the given flags
std::vector<EffectInfo> getAllEffects(const std::vector<Item> &Items,
                                      CapabilityFlags Flags) {
  std::vector<EffectInfo> AllEffects = Items.front().getAllEffects();
  for (std::size_t Idx = 1; Idx < Items.size(); ++Idx) {
    auto &It = Items.at(Idx);
    for (const auto &Info : It.getAllEffects()) {
      // We allow merging of ranged, adjacent and self effects here we check
      // only that the main flag is set not the sub-flags for range, etc.
      if (Info.Attributes.Flags & Flags) {
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

  // Keep track of the overall flags of all effects
  CapabilityFlags AllEffectFlags = CapabilityFlags::None;

  for (const auto &Info : AllEffects) {
    auto Effect = Info.Effect.get();

    // Handle NullEffect separately, we only keep the effect if no other
    // effect has the same flags
    if (dynamic_cast<NullEffect *>(Info.Effect.get())) {
      if (NullInfo) {
        NullInfo->Attributes.Flags |= Info.Attributes.Flags;
      } else {
        NullInfo = Info;
      }
      continue;
    }

    // Handle RemoveEffectBase separately, we remove all newly added effects
    // that are removed by this effect
    if (auto *RM = dynamic_cast<const RemoveEffectBase *>(Effect)) {
      auto RMFlags = Info.Attributes.Flags;
      auto It = std::remove_if(NewEffects.begin(), NewEffects.end(),
                               [RM, RMFlags](const EffectInfo &Info) {
                                 if (Info.Attributes.Flags != RMFlags) {
                                   return false;
                                 }
                                 return RM->removesEffect(*Info.Effect);
                               });
      NewEffects.erase(It, NewEffects.end());
      continue;
    }

    // Update all flags with the effect info flags
    AllEffectFlags |= Info.Attributes.Flags;

    // Try to combine the effect with any existing effect
    for (auto &NewInfo : NewEffects) {
      if (NewInfo.Attributes.Flags != Info.Attributes.Flags) {
        continue;
      }
      auto NewEffect = NewInfo.Effect.get();
      if (NewEffect->canAddFrom(*Effect)) {
        NewEffect->addFrom(*Effect);
        NewInfo.Attributes.updateCostsFrom(Info.Attributes);
        Effect = nullptr;
        break;
      }
    }

    // If effect was not combined with any existing effect, add it as a new one
    if (Effect) {
      NewEffects.push_back({Info.Attributes, Effect->clone()});
    }
  }

  // Add NullEffect if there are any flags that are not covered by any other
  // effect
  if (NullInfo && (NullInfo->Attributes.Flags & ~AllEffectFlags)) {
    NewEffects.push_back(*NullInfo);
  }
  return NewEffects;
}

} // namespace

Item CraftingHandler::craftEnhancedItem(const std::vector<Item> &Items) const {
  auto &First = Items.at(0);
  auto Flags = First.getCapabilityFlags();

  // Create new item prototype to use as the overriding item specialization, we
  // explicitly copy all effects from the first item (including the
  // specialization effects). The item specialization will not be copied (the
  // first item is already specialized).
  auto Special = std::make_shared<ItemPrototype>(
      ItemProtoId(-1), First.getName(), First.getDescription(), First.getType(),
      First.getMaxStackSize(), std::vector<EffectInfo>{});

  // Combine effects from all other items
  Special->Effects = getAllEffects(Items, Flags);
  Special->Effects = computeNewEffects(Special->Effects);

  // Create the new item
  return Item(First.getProto(), /*StackSize=*/1, Special,
              /*SpecOverrides=*/true);
}

} // namespace rogue