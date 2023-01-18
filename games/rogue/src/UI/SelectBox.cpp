#include <rogue/UI/Frame.h>
#include <rogue/UI/SelectBox.h>

namespace rogue::ui {

std::string SelectBox::Option::getValue() const {
  auto Value = "[ ] " + Text;
  Value[1] = ShortCut;
  return Value;
}

cxxg::types::Size SelectBox::getSize(const std::vector<Option> &Options) {
  std::size_t Width = 0;
  for (const auto &Opt : Options) {
    Width = std::max(Width, Opt.getValue().size());
  }
  // +2 for frame
  return {Width + 2, Options.size() + 2};
}

SelectBox::SelectBox(cxxg::types::Position Pos,
                     const std::vector<Option> &Options)
    : BaseRect(Pos, getSize(Options)), Options(Options) {
  ItSel = std::make_shared<ItemSelect>(Pos);
  cxxg::types::Position StartPos = {1, 1};
  for (auto const &Opt : Options) {
    ItSel->addSelect<Select>(Opt.getValue(), StartPos, Size.X);
    StartPos.Y++;
  }
  Decorated = std::make_shared<Frame>(ItSel, Pos, Size);
}

void SelectBox::registerOnSelectCallback(
    ItemSelect::OnSelectCallback OnSelectCb) {
  ItSel->registerOnSelectCallback(std::move(OnSelectCb));
}

void SelectBox::setPos(cxxg::types::Position P) {
  Decorated->setPos(P);
  Pos = P;
}

bool SelectBox::handleInput(int Char) {
  for (std::size_t Idx = 0; Idx < Options.size(); Idx++) {
    if (Options.at(Idx).ShortCut == Char) {
      ItSel->select(Idx);
      ItSel->handleSelect();
      return true;
    }
  }
  return ItSel->handleInput(Char);
}

std::string_view SelectBox::getInteractMsg() const { return "FIXME"; }

void SelectBox::draw(cxxg::Screen &Scr) const { Decorated->draw(Scr); }

} // namespace rogue::ui