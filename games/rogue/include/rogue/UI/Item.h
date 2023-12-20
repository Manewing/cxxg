#ifndef ROGUE_UI_ITEM_H
#define ROGUE_UI_ITEM_H

#include <cxxg/Types.h>
#include <rogue/ItemType.h>
#include <vector>

namespace rogue {
class Item;
}

namespace rogue::ui {

cxxg::types::TermColor getColorForItemType(ItemType Type);

std::string getEffectDescription(const EffectInfo &EffInfo);

std::string getCapabilityDescription(const std::vector<EffectInfo> &AllEffects,
                                     CapabilityFlags Flag);

std::string getItemEffectDescription(const Item &It);

std::string getItemText(const Item &It);

} // namespace rogue::ui

#endif // ROGUE_UI_ITEM_H