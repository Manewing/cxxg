#include <cxxg/Screen.h>
#include <iomanip>
#include <rogue/Components/Stats.h>
#include <rogue/UI/CompHelpers.h>

namespace rogue::ui {

cxxg::types::RgbColor getHealthColor(int Health, int MaxHealth) {
  if (Health <= 0) {
    return {145, 10, 10};
  }
  int Percent = (Health * 100) / MaxHealth;
  if (Percent >= 66) {
    return {135, 250, 10};
  }
  if (Percent >= 33) {
    return {225, 140, 10};
  }
  return {245, 30, 30};
}

void addHealthInfo(cxxg::Screen &Scr, cxxg::types::Position Pos,
                   entt::registry &Reg, entt::entity Entity) {

  if (const auto *HC = Reg.try_get<HealthComp>(Entity)) {
    const auto HealthColor = getHealthColor(HC->Value, HC->MaxValue);
    const auto MaxHealthColor = getHealthColor(HC->MaxValue, HC->MaxValue);
    Scr[Pos.Y][Pos.X] << "Health  : HP: " << HealthColor << std::fixed
                      << std::setprecision(1) << std::setw(6) << HC->Value
                      << cxxg::types::Color::NONE << " / " << MaxHealthColor
                      << std::setprecision(1) << std::setw(9) << HC->MaxValue;
  } else {
    Scr[Pos.Y][Pos.X] << "Health  : HP:    ---";
  }
}

cxxg::types::RgbColor getManaColor(int Mana, int MaxMana) {
  if (Mana <= 0) {
    return {145, 145, 145};
  }
  int Percent = (Mana * 100) / MaxMana;
  if (Percent >= 66) {
    return {10, 140, 250};
  }
  if (Percent >= 33) {
    return {80, 150, 200};
  }
  return {145, 145, 200};
}

void addManaInfo(cxxg::Screen &Scr, cxxg::types::Position Pos,
                 entt::registry &Reg, entt::entity Entity) {

  if (const auto *MC = Reg.try_get<ManaComp>(Entity)) {
    auto ManaColor = getManaColor(MC->Value, MC->MaxValue);
    auto MaxManaColor = getManaColor(MC->MaxValue, MC->MaxValue);
    Scr[Pos.Y][Pos.X] << "Mana    : MP: " << ManaColor << std::fixed
                      << std::setprecision(1) << std::setw(6) << MC->Value
                      << cxxg::types::Color::NONE << " / " << MaxManaColor
                      << std::setprecision(1) << std::setw(9) << MC->MaxValue;
  } else {
    Scr[Pos.Y][Pos.X] << "Mana    : MP:    ---";
  }
}

void addAgilityInfo(cxxg::Screen &Scr, cxxg::types::Position Pos,
                    entt::registry &Reg, entt::entity Entity) {

  if (const auto *AC = Reg.try_get<AgilityComp>(Entity)) {
    Scr[Pos.Y][Pos.X] << "Agility : AP: " << std::fixed << std::setprecision(1)
                      << std::setw(6) << AC->AP
                      << " | AG: " << std::setprecision(1) << std::setw(5)
                      << AC->Agility;
  } else {
    Scr[Pos.Y][Pos.X] << "AG:    --- | AG:   ---";
  }
}

} // namespace rogue::ui
