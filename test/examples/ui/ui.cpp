#include <cxxg/Game.h>
#include <cxxg/Screen.h>
#include <cxxg/Utils.h>

#include <rogue/UI/Decorator.h>
#include <rogue/UI/Frame.h>
#include <rogue/UI/ItemSelect.h>
#include <rogue/UI/ListSelect.h>

using namespace rogue::ui;
using Size = cxxg::types::Size;
using Position = cxxg::types::Position;

class ExitHandlerProvider {
public:
  using ExitHandlerCallback = std::function<void()>;

public:
  virtual ~ExitHandlerProvider() = default;

  void registerExitHandler(ExitHandlerCallback EMC) {
    this->EMC = std::move(EMC);
  }

protected:
  void handleExit() { EMC(); }

private:
  ExitHandlerCallback EMC;
};

// PreventCloseDecorator
// PreventResizeDecorator

class MoveDecorator : public Decorator, public ExitHandlerProvider {
public:
  static constexpr auto IconColor = cxxg::types::RgbColor{135, 250, 10};

public:
  explicit MoveDecorator(const std::shared_ptr<Widget> &Comp)
      : Decorator(Comp->getPos(), Comp) {}

  bool handleInput(int Char) final {
    switch (Char) {
    case cxxg::utils::KEY_ESC:
      handleExit();
      break;
    case cxxg::utils::KEY_LEFT:
      setPos(Pos + Position{-1, 0});
      break;
    case cxxg::utils::KEY_RIGHT:
      setPos(Pos + Position{1, 0});
      break;
    case cxxg::utils::KEY_UP:
      setPos(Pos + Position{0, -1});
      break;
    case cxxg::utils::KEY_DOWN:
      setPos(Pos + Position{0, 1});
      break;
    default:
      break;
    }
    return true;
  }

  void draw(cxxg::Screen &Scr) const final {
    Decorator::draw(Scr);
    Scr[Pos.Y][Pos.X] << IconColor << "X";
  }

  void setComp(std::shared_ptr<Widget> Comp) { this->Comp = std::move(Comp); }
};

class WindowContainer : public Widget {
public:
  static constexpr int KEY_MOVE = 'm';
  static constexpr int KEY_NEXT_WINDOW = 'C';
  static constexpr int KEY_PREV_WINDOW = 'V';
  static constexpr int KEY_AUTO_LAYOUT = 'X';

  static constexpr auto ActiveColor = cxxg::types::RgbColor{90, 130, 175};

public:
  WindowContainer() : Widget({0, 0}) {}

  bool handleInput(int Char) override {
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
  std::string_view getInteractMsg() const override {
    if (hasActiveWindow()) {
      return getActiveWindow().getInteractMsg();
    }
    return "";
  }

  void draw(cxxg::Screen &Scr) const override {
    for (const auto &Window : Windows) {
      if (Window.get() != &getActiveWindow()) {
        Window->draw(Scr);
      }
    }
    if (hasActiveWindow()) {
      const auto &Wdw = getActiveWindow();
      Wdw.draw(Scr);
      //
      //    ,_
      //    |[List] [Item] [Tooltip]
      //
      //    ,_
      //    |+--------
      //     |
      Scr[Wdw.getPos() - Position{1, 1}] = '_';
      Scr[Wdw.getPos() - Position{1, 1}] = ActiveColor;
      Scr[Wdw.getPos() - Position{0, 1}] = ',';
      Scr[Wdw.getPos() - Position{0, 1}] = ActiveColor;
      Scr[Wdw.getPos() - Position{1, 0}] = '|';
      Scr[Wdw.getPos() - Position{1, 0}] = ActiveColor;
    }
  }

  bool hasActiveWindow() const { return !Windows.empty(); }

  Widget &getActiveWindow() { return *Windows.at(FocusIdx); }

  const Widget &getActiveWindow() const { return *Windows.at(FocusIdx); }

  std::shared_ptr<Widget> &getActiveWindowPtr() { return Windows.at(FocusIdx); }

  /// @brief Closes the active window if there is one
  /// @return True if there is still is a window left, otherwise false
  bool closeActiveWindow() {
    if (Windows.empty()) {
      return false;
    }
    Windows.erase(Windows.begin() + FocusIdx);
    FocusIdx = 0;
    return !Windows.empty();
  }

  void addWindow(std::shared_ptr<Widget> Window) {
    Windows.emplace_back(std::move(Window));
    FocusIdx = Windows.size() - 1;
  }

  void selectWindow(std::size_t Idx) {
    FocusIdx = Idx;
    if (FocusIdx >= Windows.size()) {
      FocusIdx = 0;
    }
  }

  void selectNextWindow() { selectWindow(FocusIdx + 1); }

  void selectPrevWindow() {
    if (FocusIdx == 0) {
      selectWindow(Windows.size() - 1);
      return;
    }
    selectWindow(FocusIdx - 1);
  }

  void autoLayoutWindows(cxxg::types::Position StartPos,
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

protected:
  bool exitMoveActiveWindow() {
    if (!MoveDeco) {
      return false;
    }
    getActiveWindowPtr() = MoveDeco->getComp();
    MoveDeco.reset();
    return true;
  }

  bool enterMoveActiveWindow() {
    if (!hasActiveWindow()) {
      return false;
    }
    MoveDeco.reset(new MoveDecorator(getActiveWindowPtr()));
    getActiveWindowPtr() = MoveDeco;
    MoveDeco->registerExitHandler([this]() { exitMoveActiveWindow(); });
    return true;
  }

  void switchMoveActiveWindow(bool IsMoving) {
    if (!IsMoving) {
      exitMoveActiveWindow();
      return;
    }
    enterMoveActiveWindow();
  }

private:
  std::size_t FocusIdx = 0;
  std::shared_ptr<MoveDecorator> MoveDeco = nullptr;
  std::vector<std::shared_ptr<Widget>> Windows;
};

class UITest : public cxxg::Game {
public:
  explicit UITest(cxxg::Screen &Scr) : cxxg::Game(Scr) {
    auto UISelect = std::make_shared<ItemSelect>(Position{0, 0});
    UISelect->addSelect<Select>("[List]", Position{0, 0}, 6);
    UISelect->addSelect<Select>("[Item]", Position{7, 0}, 6);
    UISelect->addSelect<Select>("[Tooltip]", Position{14, 0}, 9);
    UISelect->select(0);
    UISelect->registerOnSelectCallback(
        [this](Select &Selected) { switchUI(Selected.getValue()); });
    WdwContainer.addWindow(UISelect);
  }

  void initialize(bool BufferedInput = false, unsigned TickDelayUs = 0) final {
    cxxg::Game::initialize(BufferedInput, TickDelayUs);
    handleDraw();
  }

  void run(bool Blocking = true) final { cxxg::Game::run(Blocking); }

  bool handleInput(int Char) final {
    WdwContainer.handleInput(Char);
    return true;
  }

  void handleDraw() final {
    WdwContainer.draw(Scr);
    cxxg::Game::handleDraw();
  }

private:
  void switchUI(const std::string &UIName) {
    if (UIName == "[List]") {
      WdwContainer.addWindow(createListUI());
    } else if (UIName == "[Item]") {
      //      ActiveWidget = createItemUI();
    } else if (UIName == "[Tooltip]") {
      //      ActiveWidget = createTooltipUI();
    }
  }

  static std::shared_ptr<Widget> createListUI() {
    Position P{2 + rand() % 4, 2 + rand() % 4};
    Size S{static_cast<std::size_t>(18 + rand() % 8),
           static_cast<std::size_t>(5 + rand() % 5)};
    auto LS = std::make_shared<ListSelect>(P, S);
    LS->setElements({"1. Item", "2. Item", "3. Item"});
    return std::make_shared<Frame>(LS, P, S, "List");
  }

private:
  WindowContainer WdwContainer;
};

void runUITest(cxxg::Screen &Scr) {
  UITest UI(Scr);
  UI.initialize();
  UI.run();
}

int main() {
  cxxg::Screen Scr(cxxg::Screen::getTerminalSize());
  cxxg::utils::registerSigintHandler([]() { exit(0); });

  try {
    runUITest(Scr);
  } catch (std::exception const &E) {
    std::cerr << "ERROR: " << E.what() << std::endl;
    return 1;
  }

  return 0;
}