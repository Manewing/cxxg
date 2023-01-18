#include <cxxg/Screen.h>
#include <rogue/UI/WindowContainer.h>

namespace rogue::ui {

bool WindowContainer::handleInput(int Char) {
  switch (Char) {
  case KEY_MOVE:
    switchMoveActiveWindow(!MoveDeco);
    break;
  case KEY_NEXT_WINDOW: {
    bool WasMoving = exitMoveActiveWindow();
    selectNextWindow();
    switchMoveActiveWindow(WasMoving);
  } break;
  case KEY_PREV_WINDOW: {
    bool WasMoving = exitMoveActiveWindow();
    selectPrevWindow();
    switchMoveActiveWindow(WasMoving);
  } break;
  case KEY_AUTO_LAYOUT:
    autoLayoutWindows({0, 0}, {80, 24});
    break;
  default:
    break;
  }

  if (hasActiveWindow()) {
    if (!getActiveWindow().handleInput(Char)) {
      return closeActiveWindow();
    }
  }
  return false;
}

std::string_view WindowContainer::getInteractMsg() const {
  if (hasActiveWindow()) {
    return getActiveWindow().getInteractMsg();
  }
  return "";
}

void WindowContainer::draw(cxxg::Screen &Scr) const {
  for (const auto &Window : Windows) {
    if (Window.get() != &getActiveWindow()) {
      Window->draw(Scr);
    }
  }
  if (hasActiveWindow()) {
    const auto &Wdw = getActiveWindow();
    Wdw.draw(Scr);

    //    ,_
    //    |+--------+
    //     |        |
    //     +--------+
    //
    Scr[Wdw.getPos() - cxxg::types::Position{1, 1}] = '_';
    Scr[Wdw.getPos() - cxxg::types::Position{1, 1}] = ActiveColor;
    Scr[Wdw.getPos() - cxxg::types::Position{0, 1}] = ',';
    Scr[Wdw.getPos() - cxxg::types::Position{0, 1}] = ActiveColor;
    Scr[Wdw.getPos() - cxxg::types::Position{1, 0}] = '|';
    Scr[Wdw.getPos() - cxxg::types::Position{1, 0}] = ActiveColor;
  }
}

bool WindowContainer::hasActiveWindow() const { return !Windows.empty(); }

Widget &WindowContainer::getActiveWindow() { return *Windows.at(FocusIdx); }

const Widget &WindowContainer::getActiveWindow() const {
  return *Windows.at(FocusIdx);
}

std::shared_ptr<Widget> &WindowContainer::getActiveWindowPtr() {
  return Windows.at(FocusIdx);
}

bool WindowContainer::closeActiveWindow() {
  if (Windows.empty()) {
    return false;
  }
  Windows.erase(Windows.begin() + FocusIdx);
  FocusIdx = 0;
  return !Windows.empty();
}

void WindowContainer::addWindow(std::shared_ptr<Widget> Window) {
  Windows.emplace_back(std::move(Window));
  FocusIdx = Windows.size() - 1;
}

void WindowContainer::selectWindow(std::size_t Idx) {
  FocusIdx = Idx;
  if (FocusIdx >= Windows.size()) {
    FocusIdx = 0;
  }
}

void WindowContainer::selectNextWindow() { selectWindow(FocusIdx + 1); }

void WindowContainer::selectPrevWindow() {
  if (FocusIdx == 0) {
    selectWindow(Windows.size() - 1);
    return;
  }
  selectWindow(FocusIdx - 1);
}

void WindowContainer::autoLayoutWindows(cxxg::types::Position StartPos,
                                        cxxg::types::Size Size) {
  struct WindowInfo {
    cxxg::types::Position Pos{};
    cxxg::types::Size Size{};
    unsigned long Area = 0;
    Widget *Wdw = nullptr;
  };
  if (Windows.empty()) {
    return;
  }
  std::vector<WindowInfo> WdwInfos;
  WdwInfos.reserve(Windows.size());

  for (auto &Wdw : Windows) {
    auto *W = Wdw.get();
    while (auto *Next = dynamic_cast<Decorator *>(W)) {
      W = Next->getComp().get();
    }
    if (auto *BrWdw = dynamic_cast<BaseRect *>(W)) {
      auto Size = BrWdw->getSize();
      WdwInfos.push_back(WindowInfo{BrWdw->getPos(), BrWdw->getSize(),
                                    Size.X * Size.Y, Wdw.get()});
    }
  }
  std::sort(WdwInfos.begin(), WdwInfos.end(),
            [](const auto &A, const auto &B) { return A.Area > B.Area; });

  auto PlacePos = StartPos;
  //    auto *LastWdwInfo = &WdwInfos.at(0);
  unsigned Height = WdwInfos.at(0).Size.Y;
  for (auto &WdwInfo : WdwInfos) {

    if (PlacePos.X + WdwInfo.Size.X > Size.X) {
      if (PlacePos.Y + WdwInfo.Size.Y > Size.Y) {
        // abort, can't handle
        return;
      }
      // FIXME adding the current height does not make sense
      PlacePos = {0, PlacePos.Y + static_cast<int>(Height)};
      Height = WdwInfo.Size.Y;
    }

    WdwInfo.Wdw->setPos(PlacePos);

    PlacePos.X += static_cast<int>(WdwInfo.Size.X + 1);
  }
}

bool WindowContainer::exitMoveActiveWindow() {
  if (!MoveDeco) {
    return false;
  }
  getActiveWindowPtr() = MoveDeco->getComp();
  MoveDeco.reset();
  return true;
}

bool WindowContainer::enterMoveActiveWindow() {
  if (!hasActiveWindow()) {
    return false;
  }
  MoveDeco.reset(new MoveDecorator(getActiveWindowPtr()));
  getActiveWindowPtr() = MoveDeco;
  MoveDeco->registerExitHandler([this]() { exitMoveActiveWindow(); });
  return true;
}

void WindowContainer::switchMoveActiveWindow(bool IsMoving) {
  if (!IsMoving) {
    exitMoveActiveWindow();
    return;
  }
  enterMoveActiveWindow();
}

} // namespace rogue::ui