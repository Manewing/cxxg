#include <cxxg/Game.h>
#include <cxxg/Screen.h>
#include <cxxg/Utils.h>

#include <rogue/UI/Decorator.h>
#include <rogue/UI/Frame.h>
#include <rogue/UI/ItemSelect.h>
#include <rogue/UI/ListSelect.h>
#include <rogue/UI/SelectBox.h>
#include <rogue/UI/TextBox.h>
#include <rogue/UI/WindowContainer.h>

using namespace rogue::ui;
using Size = cxxg::types::Size;
using Position = cxxg::types::Position;

class UITest : public cxxg::Game {
public:
  explicit UITest(cxxg::Screen &Scr)
      : cxxg::Game(Scr), WdwContainer({0, 0}, Scr.getSize()) {
    auto UISelect = std::make_shared<ItemSelect>(Position{0, 0});
    UISelect->addSelect<Select>("[List]", Position{0, 0}, 6);
    UISelect->addSelect<Select>("[Item]", Position{7, 0}, 6);
    UISelect->addSelect<Select>("[SelBox]", Position{14, 0}, 8);
    UISelect->addSelect<Select>("[Tooltip]", Position{23, 0}, 9);
    UISelect->select(0);
    UISelect->registerOnSelectCallback(
        [this](const Select &Selected) { switchUI(Selected.getValue()); });
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
    const auto Size = Scr.getSize();
    for (std::size_t Idx = 0; Idx < Size.Y; Idx++) {
      Scr[Idx][0] << cxxg::types::RgbColor{20, 20, 20}
                  << std::string(Size.X, '#');
    }
    WdwContainer.draw(Scr);
    cxxg::Game::handleDraw();
  }

private:
  void switchUI(const std::string &UIName) {
    if (UIName == "[List]") {
      WdwContainer.addWindow(createListUI());
    } else if (UIName == "[SelBox]") {
      WdwContainer.addWindow(createSelectBox());
    } else if (UIName == "[Item]") {
      //      ActiveWidget = createItemUI();
    } else if (UIName == "[Tooltip]") {
      WdwContainer.addWindow(createTooltipUI());
    }
  }

  std::shared_ptr<Widget> createListUI() {
    Position P{2 + rand() % 10, 2 + rand() % 10};
    Size S{static_cast<std::size_t>(18 + rand() % 8),
           static_cast<std::size_t>(5 + rand() % 5)};
    auto LS = std::make_shared<ListSelect>(P, S);
    LS->setElements(
        {{"1. Item", cxxg::types::Color::RED}, {"2. Item"}, {"3. Item"}});
    return std::make_shared<Frame>(LS, P, S, "List");
  }

  std::shared_ptr<Widget> createSelectBox() {
    Position P{2 + rand() % 10, 2 + rand() % 10};
    std::vector<SelectBox::Option> Options = {
        {'u', "Use"}, {'e', "Equip"}, {'c', "Craft"}};
    auto SB = std::make_shared<SelectBox>(P, Options);
    SB->registerOnSelectCallback([this](const auto &S) {
      WdwContainer.addWindow(
          createTextBox("You selected the option " + S.getValue()));
    });
    return SB;
  }

  std::shared_ptr<Widget> createTextBox(const std::string &Text) {
    Position P{2 + rand() % 10, 2 + rand() % 10};
    Size S{static_cast<std::size_t>(38 + rand() % 8),
           static_cast<std::size_t>(5 + rand() % 5)};
    auto TB = std::make_shared<TextBox>(P, S, Text);
    return std::make_shared<Frame>(TB, P, S, "Text");
  }

  std::shared_ptr<Widget> createTooltipUI() {
    std::string Text =
        "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam "
        "nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam "
        "erat, sed diam voluptua. At vero eos et accusam et justo duo dolores "
        "et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est "
        "Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur "
        "sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et "
        "dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam "
        "et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea "
        "takimata sanctus est Lorem ipsum dolor sit amet.";
    return createTextBox(Text);
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