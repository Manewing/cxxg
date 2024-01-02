#ifndef ROGUE_UI_COMP_HELPERS_H
#define ROGUE_UI_COMP_HELPERS_H

#include <cxxg/Types.h>
#include <entt/entt.hpp>

namespace cxxg {
class Screen;
}

namespace rogue::ui {

/// Returns color for health bar, based on current health and max health. From
/// red to green (full life).
cxxg::types::RgbColor getHealthColor(int Health, int MaxHealth);

void addHealthInfo(cxxg::Screen &Scr, cxxg::types::Position Pos,
                   entt::registry &Reg, entt::entity Entity);

/// Returns color for mana bar, based on current mana and max mana. From grey to
/// blue (full mana).
cxxg::types::RgbColor getManaColor(int Mana, int MaxMana);

void addManaInfo(cxxg::Screen &Scr, cxxg::types::Position Pos,
                 entt::registry &Reg, entt::entity Entity);

void addAgilityInfo(cxxg::Screen &Scr, cxxg::types::Position Pos,
                    entt::registry &Reg, entt::entity Entity);

} // namespace rogue::ui

#endif // ROGUE_UI_COMP_HELPERS_H