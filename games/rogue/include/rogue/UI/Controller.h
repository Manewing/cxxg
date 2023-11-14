#ifndef ROGUE_UI_CONTROLLER_H
#define ROGUE_UI_CONTROLLER_H

#include <entt/entt.hpp>
#include <memory>
#include <rogue/EventHub.h>
#include <rogue/UI/TargetUI.h>
#include <rogue/UI/Widget.h>
#include <rogue/UI/WindowContainer.h>
#include <string_view>
#include <ymir/Types.hpp>

namespace cxxg {
class Screen;
}

namespace rogue {
class History;
class Inventory;
class Equipment;
struct StatsComp;
class Level;
} // namespace rogue

namespace rogue::ui {

class Controller : public EventHubConnector {
public:
  struct PlayerInfo {
    int Health;
    int MaxHealth;
    int AP;
    std::string InteractStr;
  };

  struct TargetInfo {
    std::string Name;
    int Health;
    int MaxHealth;
  };

public:
  Controller(cxxg::Screen &Scr);
  void draw(int LevelIdx, const PlayerInfo &PI,
            const std::optional<TargetInfo> &TI);
  bool isUIActive() const;
  void handleInput(int Char);

  void addWindow(std::shared_ptr<Widget> Wdw, bool AutoLayoutWindows = false);

  void setEquipmentUI(entt::entity Entity, entt::registry &Reg);
  bool hasEquipmentUI() const;
  void closeEquipmentUI();

  void setInventoryUI(entt::entity Entity, entt::registry &Reg);
  bool hasInventoryUI() const;
  void closeInventoryUI();

  /// Opens stats UI for the selected entity
  void setStatsUI(entt::entity Entity, entt::registry &Reg);
  bool hasStatsUI() const;
  void closeStatsUI();

  /// Creates a buff UI for the selected entity
  void setBuffUI(entt::entity Entity, entt::registry &Reg);
  bool hasBuffUI() const;
  void closeBuffUI();

  /// Creates a loot UI
  /// \param Entity The entity that is looting
  /// \param InvEt The entity that is being looted
  /// \param Reg The registry
  void setLootUI(entt::entity Entity, entt::entity InvEt, entt::registry &Reg);
  bool hasLootUI() const;
  void closeLootUI();

  void setHistoryUI(History &Hist);
  bool hasHistoryUI() const;
  void closeHistoryUI();

  /// Create target UI
  /// \param TargetPos The position to target
  /// \param Lvl The level
  void setTargetUI(entt::entity SrcEt, ymir::Point2d<int> TargetPos, Level &Lvl,
                   const TargetUI::SelectTargetCb &Callback);
  bool hasTargetUI() const;
  void closeTargetUI();

  void setInteractUI(entt::entity SrcEt, ymir::Point2d<int> StartPos,
                     Level &Lvl);
  bool hasInteractUI() const;
  void closeInteractUI();

  /// Closes all windows
  void closeAll();

private:
  cxxg::Screen &Scr;
  WindowContainer WdwContainer;
};

} // namespace rogue::ui

#endif // #ifndef ROGUE_UI_CONTROLLER_H