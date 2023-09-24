#include <cxxg/Screen.h>
#include <optional>
#include <rogue/UI/Frame.h>
#include <rogue/UI/Widget.h>
#include <rogue/UI/WindowContainer.h>
#include <variant>
#include <ymir/Types.hpp>

namespace rogue::ui {

std::optional<WindowContainer::WindowInfo>
WindowContainer::WindowInfo::getWindowInfo(Widget *Wdw) {
  while (auto *Next = dynamic_cast<Decorator *>(Wdw)) {
    if (auto *BrWdw = dynamic_cast<BaseRectDecorator *>(Wdw)) {
      auto Size = BrWdw->getSize();
      return WindowInfo{BrWdw->getPos(), BrWdw->getSize(), Size.X * Size.Y,
                        Wdw};
    }

    Wdw = Next->getComp().get();
  }

  if (auto *BrWdw = dynamic_cast<BaseRect *>(Wdw)) {
    auto Size = BrWdw->getSize();
    return WindowInfo{BrWdw->getPos(), BrWdw->getSize(), Size.X * Size.Y, Wdw};
  }

  return std::nullopt;
}

WindowContainer::WindowContainer(cxxg::types::Position Pos,
                                 cxxg::types::Size Size)
    : Widget(Pos), Size(Size) {}

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
    autoLayoutWindows();
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

std::string WindowContainer::getInteractMsg() const {
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

bool WindowContainer::closeWindow(std::size_t Idx) {
  if (Windows.empty()) {
    return false;
  }
  Windows.erase(Windows.begin() + Idx);
  FocusIdx = 0;
  selectWindow(PrevFocusIdx);
  return !Windows.empty();
}

bool WindowContainer::closeWindow(Widget *Wdw) {
  for (std::size_t Idx = 0; Idx < Windows.size(); ++Idx) {
    if (Windows.at(Idx).get() == Wdw) {
      return closeWindow(Idx);
    }
  }
  return false;
}

bool WindowContainer::closeActiveWindow() { return closeWindow(FocusIdx); }

void WindowContainer::addWindow(std::shared_ptr<Widget> Window) {
  Windows.emplace_back(std::move(Window));
  selectWindow(Windows.size() - 1);
}

namespace {

Frame *getFrameFromWidget(Widget *W) {
  if (auto *F = dynamic_cast<Frame *>(W)) {
    return F;
  }
  while (auto *Next = dynamic_cast<Decorator *>(W)) {
    W = Next->getComp().get();
    if (auto *F = dynamic_cast<Frame *>(W)) {
      return F;
    }
  }
  return nullptr;
}

void changeWindowHighlight(Widget &W, bool Highlight) {
  if (auto *F = getFrameFromWidget(&W)) {
    if (Highlight) {
      F->setFrameColor(cxxg::types::RgbColor{255, 255, 200}.bold());
    } else {
      F->setFrameColor(cxxg::types::Color::NONE);
    }

    if (const auto HC = F->getHeaderColor();
        std::holds_alternative<cxxg::types::RgbColor>(HC)) {
      F->setHeaderColor(std::get<cxxg::types::RgbColor>(HC).bold(Highlight));
    }
  }
}

} // namespace

void WindowContainer::selectWindow(std::size_t Idx) {
  if (FocusIdx < Windows.size()) {
    changeWindowHighlight(*Windows.at(FocusIdx), false);
  }

  PrevFocusIdx = FocusIdx;
  FocusIdx = Idx;
  if (FocusIdx >= Windows.size()) {
    FocusIdx = 0;
  }

  if (!Windows.empty()) {
    changeWindowHighlight(*Windows.at(FocusIdx), true);
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

void WindowContainer::autoLayoutWindows() { autoLayoutWindows(Pos, Size); }

namespace {

ymir::Rect2d<unsigned long> getRect(const WindowContainer::WindowInfo &WdwInfo,
                                    bool Spacing = false) {
  return {
      {static_cast<unsigned long>(WdwInfo.Pos.X),
       static_cast<unsigned long>(WdwInfo.Pos.Y)},
      {WdwInfo.Size.X + (Spacing ? 1 : 0), WdwInfo.Size.Y + (Spacing ? 1 : 0)}};
}

bool isOverlapping(const WindowContainer::WindowInfo &WdwInfo,
                   const std::vector<WindowContainer::WindowInfo> &WdwInfos) {
  const auto ThisRect = getRect(WdwInfo);
  for (const auto &Other : WdwInfos) {
    if (WdwInfo.Wdw == Other.Wdw || (Other.Pos.X == -1 && Other.Pos.Y == -1)) {
      continue;
    }
    if (ThisRect.overlaps(getRect(Other, true))) {
      return true;
    }
  }
  return false;
}

std::optional<cxxg::types::Position> findPositionForWindow(
    cxxg::types::Position StartPos, cxxg::types::Size Size,
    WindowContainer::WindowInfo &WdwInfo,
    const std::vector<WindowContainer::WindowInfo> &WdwInfos) {
  for (unsigned long Y = StartPos.Y; Y < StartPos.Y + Size.Y; ++Y) {
    for (unsigned long X = StartPos.X; X < StartPos.X + Size.X; ++X) {
      if (X + WdwInfo.Size.X > StartPos.X + Size.X ||
          Y + WdwInfo.Size.Y > StartPos.Y + Size.Y) {
        continue;
      }
      WdwInfo.Pos = {static_cast<int>(X), static_cast<int>(Y)};
      if (!isOverlapping(WdwInfo, WdwInfos)) {
        return WdwInfo.Pos;
      }
    }
  }
  WdwInfo.Pos = {-1, -1};
  return std::nullopt;
}

} // namespace

void WindowContainer::autoLayoutWindows(cxxg::types::Position StartPos,
                                        cxxg::types::Size Size) {
  if (Windows.empty()) {
    return;
  }
  std::vector<WindowInfo> WdwInfos;
  WdwInfos.reserve(Windows.size());

  for (auto &Wdw : Windows) {
    if (auto WI = WindowInfo::getWindowInfo(Wdw.get())) {
      WdwInfos.push_back(*WI);
    }
  }

  if (WdwInfos.empty()) {
    return;
  }

  std::sort(WdwInfos.begin(), WdwInfos.end(),
            [](const auto &A, const auto &B) { return A.Area > B.Area; });

  // Auto layout windows based on their rectangle
  for (auto &WdwInfo : WdwInfos) {
    WdwInfo.Pos = {-1, -1};
  }
  WdwInfos.front().Pos = StartPos;
  for (auto &WdwInfo : WdwInfos) {
    if (auto Pos = findPositionForWindow(StartPos, Size, WdwInfo, WdwInfos)) {
      WdwInfo.Wdw->setPos(*Pos);
    }
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