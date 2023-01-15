#include <cxxg/Screen.h>
#include <cxxg/Utils.h>
#include <rogue/UI/ItemSelect.h>

namespace rogue::ui {

namespace {
static constexpr cxxg::types::RgbColor HighlightColor{255, 255, 255, true,
                                                      100, 100, 100};
static constexpr auto NoColor = cxxg::types::Color::NONE;
} // namespace

LabeledSelect::LabeledSelect(std::string Label, std::string Value,
                             cxxg::types::Position Pos, unsigned Width)
    : BaseRect(Pos, {Width, 1}), Label(std::move(Label)),
      Value(std::move(Value)) {}

const std::string &LabeledSelect::getLabel() const { return Label; }

void LabeledSelect::setValue(std::string NewValue) {
  Value = std::move(NewValue);
}

const std::string &LabeledSelect::getValue() const { return Value; }

void LabeledSelect::unselect() { IsSelected = false; }

void LabeledSelect::select() { IsSelected = true; }

bool LabeledSelect::isSlected() const { return IsSelected; }

bool LabeledSelect::handleInput(int Char) {
  (void)Char;
  return false;
}

std::string_view LabeledSelect::getInteractMsg() const { return ""; }

void LabeledSelect::draw(cxxg::Screen &Scr) const {
  BaseRect::draw(Scr);
  if (IsSelected) {
    Scr[Pos.Y][Pos.X] << HighlightColor << Label << ":" << NoColor << " "
                      << Value;
  } else {
    Scr[Pos.Y][Pos.X] << Label << ": " << Value;
  }
}

void ItemSelect::addSelect(std::shared_ptr<LabeledSelect> Select) {
  Select->setPos(Select->getPos() + Pos);
  Selects.emplace_back(std::move(Select));
  selectNext();
}

void ItemSelect::selectNext() {
  if (!Selects.empty()) {
    Selects.at(SelectedIdx)->unselect();
  }
  if (++SelectedIdx >= Selects.size()) {
    SelectedIdx = 0;
  }
  if (!Selects.empty()) {
    Selects.at(SelectedIdx)->select();
  }
}

void ItemSelect::selectPrev() {
  if (!Selects.empty()) {
    Selects.at(SelectedIdx)->unselect();
  }
  if (SelectedIdx-- == 0) {
    SelectedIdx = Selects.size() - 1;
  }
  if (!Selects.empty()) {
    Selects.at(SelectedIdx)->select();
  }
}

std::size_t ItemSelect::getSelectedIdx() const { return SelectedIdx; }

LabeledSelect &ItemSelect::getSelected() { return *Selects.at(SelectedIdx); }

LabeledSelect &ItemSelect::getSelect(std::size_t Idx) {
  return *Selects.at(Idx);
}

void ItemSelect::setPos(cxxg::types::Position P) {
  for (auto &Select : Selects) {
    Select->setPos(Select->getPos() - Pos + P);
  }
  Pos = P;
}

bool ItemSelect::handleInput(int Char) {
  switch (Char) {
  case cxxg::utils::KEY_ESC:
    return false;
  case cxxg::utils::KEY_UP:
    selectPrev();
    break;
  case cxxg::utils::KEY_DOWN:
    selectNext();
    break;
  default:
    break;
  }
  return true;
}

std::string_view ItemSelect::getInteractMsg() const { return "[^v] Nav."; }

void ItemSelect::draw(cxxg::Screen &Scr) const {
  for (const auto &Select : Selects) {
    Select->draw(Scr);
  }
}

} // namespace rogue::ui