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

class MoveDecorator : public Decorator {
public:
  static constexpr auto IconColor = cxxg::types::RgbColor{135, 250, 10};

public:
  explicit MoveDecorator(std::shared_ptr<Widget> &OverrideComp)
      : Decorator(OverrideComp->getPos(), OverrideComp),
        OverrideComp(OverrideComp) {}

  bool handleInput(int Char) final {
    switch (Char) {
    case cxxg::utils::KEY_ESC:
    case 'm':
      returnToComp();
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

private:
  void returnToComp() {
    auto Copy = OverrideComp;
    OverrideComp = getComp();
    (void)Copy;
  }

private:
  std::shared_ptr<Widget> &OverrideComp;
};

class UITest : public cxxg::Game {
public:
  explicit UITest(cxxg::Screen &Scr) : cxxg::Game(Scr) {
    UISelect = std::make_shared<ItemSelect>(Position{0, 0});
    UISelect->addSelect<Select>("[List]", Position{0, 0}, 6);
    UISelect->addSelect<Select>("[Item]", Position{7, 0}, 6);
    UISelect->addSelect<Select>("[Tooltip]", Position{14, 0}, 9);
    UISelect->select(0);
  }

  void initialize(bool BufferedInput = false, unsigned TickDelayUs = 0) final {
    cxxg::Game::initialize(BufferedInput, TickDelayUs);
    handleDraw();
  }

  void run(bool Blocking = true) final { cxxg::Game::run(Blocking); }

  bool handleInput(int Char) final {
    if (ActiveWidget) {
      if (Char == 'm' && !dynamic_cast<MoveDecorator *>(ActiveWidget.get())) {
        ActiveWidget.reset(new MoveDecorator(ActiveWidget));
        return true;
      }
      if (!ActiveWidget->handleInput(Char)) {
        ActiveWidget = nullptr;
      }
      return true;
    }

    switch (Char) {
    case cxxg::utils::KEY_ENTER:
      switchUI(UISelect->getSelected().getValue());
      break;
    default:
      UISelect->handleInput(Char);
      break;
    }
    return true;
  }

  void handleDraw() final {
    UISelect->draw(Scr);
    if (ActiveWidget) {
      ActiveWidget->draw(Scr);
    }
    cxxg::Game::handleDraw();
  }

private:
  void switchUI(const std::string &UIName) {
    if (UIName == "[List]") {
      ActiveWidget.reset(createListUI());
    } else if (UIName == "[Item]") {
      //      ActiveWidget = createItemUI();
    } else if (UIName == "[Tooltip]") {
      //      ActiveWidget = createTooltipUI();
    }
  }

  static Widget *createListUI() {
    Position P{4, 4};
    Size S{25, 10};
    auto LS = std::make_shared<ListSelect>(P, S);
    LS->setElements({"1. Item", "2. Item", "3. Item"});
    return new Frame(LS, P, S, "List");
  }

private:
  std::shared_ptr<ItemSelect> UISelect;
  std::shared_ptr<Widget> ActiveWidget = nullptr;
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