#ifndef ROGUE_UI_CONTROLLER_H
#define ROGUE_UI_CONTROLLER_H

#include <entt/entt.hpp>
#include <filesystem>
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
class CraftingDatabase;
class CraftingHandler;
struct SaveGameInfo;
} // namespace rogue

namespace rogue::ui {

class Controller : public EventHubConnector {
public:
  struct SkillInfo {
    std::string Key;
    std::string Name;
    cxxg::types::TermColor NameColor;
  };

  struct PlayerInfo {
    int Health;
    int MaxHealth;
    int Mana;
    int MaxMana;
    std::string InteractStr;
    std::vector<SkillInfo> Skills;
  };

  struct TargetInfo {
    std::string Name;
    int Health;
    int MaxHealth;
  };

  using LoadGameCbTy = std::function<void(const SaveGameInfo &)>;
  using SaveGameCbTy = std::function<void(const SaveGameInfo &)>;

public:
  Controller(cxxg::Screen &Scr);
  void draw(int LevelIdx, const PlayerInfo &PI,
            const std::optional<TargetInfo> &TI);
  bool isUIActive() const;
  void handleInput(int Char);

  void addWindow(std::shared_ptr<Widget> Wdw, bool AutoLayoutWindows = false,
                 bool CenterWindow = false);

  template <typename T> T *getWindowOfType() {
    return WdwContainer.getWindowOfType<T>();
  }

  template <typename T> const T *getWindowOfType() const {
    return WdwContainer.getWindowOfType<T>();
  }

  void setMenuUI(Level &Lvl, const LoadGameCbTy &LoadGameCb,
                 const SaveGameCbTy &SaveGameCb);
  bool hasMenuUI() const;
  void closeMenuUI();

  void tooltip(std::string Text, std::string Header = "",
               bool CloseOtherWindows = false);

  void createYesNoDialog(std::string Text, const std::function<void(bool)> &Cb,
                         bool CloseOtherWindows = false);

  void setCommandLineUI(Level &Lvl);
  bool hasCommandLineUI() const;
  void closeCommandLineUI();

  void setEquipmentUI(entt::entity Entity, Level &Lvl);
  bool hasEquipmentUI() const;
  void closeEquipmentUI();

  void setInventoryUI(entt::entity Entity, Level &Lvl);
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
  /// \param Header The header to display for window
  void setLootUI(entt::entity Entity, entt::entity InvEt, Level &Lvl,
                 const std::string &Header);
  bool hasLootUI() const;
  void closeLootUI();

  /// Creates a crafting UI
  /// \param Entity The entity that is crafting
  /// \param Reg The registry
  void setCraftingUI(entt::entity Entity, entt::registry &Reg,
                     const CraftingDatabase &CraftingDb,
                     const CraftingHandler &Crafter);
  bool hasCraftingUI() const;
  void closeCraftingUI();

  void setHistoryUI(History &Hist);
  bool hasHistoryUI() const;
  void closeHistoryUI();

  /// Create target UI
  /// \param TargetPos The position to target
  /// \param Lvl The level
  void setTargetUI(ymir::Point2d<int> TargetPos, std::optional<unsigned> Range,
                   Level &Lvl, const TargetUI::SelectTargetCb &Callback);
  bool hasTargetUI() const;
  void closeTargetUI();

  void setInteractUI(entt::entity SrcEt, ymir::Point2d<int> StartPos,
                     Level &Lvl);
  bool hasInteractUI() const;
  void closeInteractUI();

  /// Closes all windows
  void closeAll();

  void handleResize(cxxg::types::Size Size);

public:
  bool DelayTicks = false;

private:
  cxxg::Screen &Scr;
  WindowContainer WdwContainer;
};

} // namespace rogue::ui

#endif // #ifndef ROGUE_UI_CONTROLLER_H