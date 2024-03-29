#include <cxxg/Screen.h>
#include <cxxg/Utils.h>
#include <rogue/Components/Items.h>
#include <rogue/Context.h>
#include <rogue/Event.h>
#include <rogue/ItemDatabase.h>
#include <rogue/Level.h>
#include <rogue/LevelGenerator.h>
#include <rogue/UI/CommandLine.h>
#include <rogue/UI/Controller.h>
#include <rogue/UI/Controls.h>
#include <rogue/UI/TargetUI.h>

namespace rogue::ui {

TextLineEdit::TextLineEdit(cxxg::types::Position Pos, unsigned Width)
    : BaseRect(Pos, {Width, 1}) {}

void TextLineEdit::draw(cxxg::Screen &Scr) const {
  BaseRect::draw(Scr);
  Scr[Pos] << Text;
  Scr[Pos.Y][Pos.X + CursorPos] =
      cxxg::types::RgbColor{0, 0, 0, true, 255, 255, 255};
}

bool TextLineEdit::handleInput(int Char) {
  switch (Char) {
  case cxxg::utils::KEY_LEFT:
    if (CursorPos >= 1) {
      --CursorPos;
    }
    return true;
  case cxxg::utils::KEY_RIGHT:
    if (CursorPos < Text.size()) {
      ++CursorPos;
    }
    return true;
  case cxxg::utils::KEY_ENTER:
    return false;
  case cxxg::utils::KEY_ESC:
    return false;
  case cxxg::utils::KEY_DEL:
    if (!Text.empty() && CursorPos > 0) {
      Text.erase(Text.begin() + CursorPos - 1);
      CursorPos--;
    }
    return true;
  case cxxg::utils::KEY_DEL_C:
    if (!Text.empty() && CursorPos < Text.size()) {
      Text.erase(Text.begin() + CursorPos);
      CursorPos = std::min(CursorPos, Text.size());
    }
    return true;
  default:
    break;
  }

  if (Char >= 32 && Char <= 126) {
    Text.insert(Text.begin() + CursorPos, Char);
    CursorPos++;
    return true;
  }
  Text += KeyOption::getCharStr(Char);
  return true;
}

void TextLineEdit::setText(const std::string &Text) {
  this->Text = Text;
  CursorPos = Text.size();
}

const std::string &TextLineEdit::getText() const { return Text; }

CommandLineController::CommandLineController(Controller &Ctrl, Level &Lvl)
    : BaseRectDecorator({0, 0}, {0, 0}, nullptr), Ctrl(Ctrl), Lvl(Lvl) {
  CmdLine = std::make_shared<TextLineEdit>(Pos, 80);
  Comp = CmdLine;
}

bool CommandLineController::handleInput(int Char) {
  if (Char == cxxg::utils::KEY_ENTER) {
    auto Text = CmdLine->getText();
    CmdLine->setText("");
    return handleCommandLine(Text);
  }
  // We are by passing the window container
  if (Char == cxxg::utils::KEY_ESC) {
    Ctrl.closeCommandLineUI();
    return false;
  }
  return BaseRectDecorator::handleInput(Char);
}

std::string CommandLineController::getInteractMsg() const {
  return "Enter command";
}

void CommandLineController::draw(cxxg::Screen &Scr) const {
  BaseRectDecorator::draw(Scr);
}

namespace {
bool startswith(const std::string_view &Str, const std::string_view &Prefix) {
  if (Str.size() < Prefix.size()) {
    return false;
  }
  return Str.substr(0, Prefix.size()) == Prefix;
}
} // namespace

bool CommandLineController::handleCommandLine(const std::string &Cmd) {
  if (Cmd == "quit") {
    Ctrl.closeCommandLineUI();
    return false;
  } else if (Cmd == "reveal") {
    Lvl.revealMap();
    publish(DebugMessageEvent() << "Map revealed");
  } else if (Cmd == "next_level") {
    publish(DebugMessageEvent()
            << "Switching to level: " + std::to_string(Lvl.getLevelId() + 1));
    publish(SwitchLevelEvent{
        {}, Lvl.getLevelId() + 1, true, entt::null, entt::null});
  } else if (Cmd == "debug_targets") {
    TargetInfo::DebugTargets = !TargetInfo::DebugTargets;
    publish(DebugMessageEvent()
            << "Debug targets: " + std::to_string(TargetInfo::DebugTargets));
  } else if (Cmd == "debug_rooms") {
    GeneratedMapLevelGenerator::DebugRooms =
        !GeneratedMapLevelGenerator::DebugRooms;
    publish(DebugMessageEvent()
            << "Debug rooms: " +
                   std::to_string(GeneratedMapLevelGenerator::DebugRooms));
  } else if (Cmd == "delay_ticks") {
    Ctrl.DelayTicks = !Ctrl.DelayTicks;
    publish(DebugMessageEvent()
            << "Delaying ticks: " + std::to_string(Ctrl.DelayTicks));
  } else if (startswith(Cmd, "give_item ")) {
    auto ItemName = Cmd.substr(10);
    publish(DebugMessageEvent() << "Giving item: " + ItemName);
    auto Player = Lvl.getPlayer();
    auto &Inv = Lvl.Reg.get<InventoryComp>(Player);
    const auto &ItemDb = Lvl.Reg.ctx().get<GameContext>().ItemDb;
    try {
      auto ItId = ItemDb.getItemId(ItemName);
      Inv.Inv.addItem(ItemDb.createItem(ItId));
    } catch (const std::exception &E) {
      publish(DebugMessageEvent()
              << "Failed to give item: " + std::string(E.what()));
    }
  } else {
    publish(DebugMessageEvent() << "Unknown command: " + Cmd);
  }
  return true;
}

} // namespace rogue::ui