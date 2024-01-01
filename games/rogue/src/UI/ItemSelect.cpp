#include <cxxg/Screen.h>
#include <cxxg/Utils.h>
#include <rogue/UI/Controls.h>
#include <rogue/UI/ItemSelect.h>

namespace rogue::ui {

namespace {
static constexpr cxxg::types::RgbColor HighlightColor{255, 255, 255, true,
                                                      100, 100, 100};
static constexpr auto NoColor = cxxg::types::Color::NONE;
} // namespace

Select::Select(std::string Value, cxxg::types::Position Pos, unsigned Width,
               cxxg::types::TermColor ValueColor)
    : BaseRect(Pos, {Width, 1}), Value(std::move(Value)),
      ValueColor(ValueColor) {}

void Select::setValue(std::string NewValue) { Value = std::move(NewValue); }

const std::string &Select::getValue() const { return Value; }

const cxxg::types::TermColor &Select::getValueColor() const {
  return ValueColor;
}

void Select::setValueColor(cxxg::types::TermColor VC) { ValueColor = VC; }

void Select::unselect() { IsSelected = false; }

void Select::select() { IsSelected = true; }

bool Select::isSlected() const { return IsSelected; }

bool Select::handleInput(int Char) {
  (void)Char;
  return false;
}

std::string Select::getInteractMsg() const { return ""; }

void Select::draw(cxxg::Screen &Scr) const {
  BaseRect::draw(Scr);
  if (IsSelected) {
    Scr[Pos.Y][Pos.X].width(Size.X) << HighlightColor << Value << NoColor;
  } else {
    Scr[Pos.Y][Pos.X].width(Size.X) << ValueColor << Value << NoColor;
  }
}

LabeledSelect::LabeledSelect(std::string Label, std::string Value,
                             cxxg::types::Position Pos, unsigned Width,
                             cxxg::types::TermColor ValueColor,
                             cxxg::types::TermColor LabelColor)
    : Select(std::move(Value), Pos, Width, ValueColor), Label(std::move(Label)),
      LabelColor(LabelColor) {}

const std::string &LabeledSelect::getLabel() const { return Label; }

std::string LabeledSelect::getInteractMsg() const { return ""; }

void LabeledSelect::draw(cxxg::Screen &Scr) const {
  BaseRect::draw(Scr);
  if (IsSelected) {
    Scr[Pos.Y][Pos.X].width(Size.X) << HighlightColor << Label << ":"
                                    << " " << ValueColor << Value << NoColor;
  } else {
    Scr[Pos.Y][Pos.X].width(Size.X)
        << LabelColor << Label << ": " << ValueColor << Value << NoColor;
  }
}

void ItemSelect::addSelect(std::shared_ptr<Select> S) {
  S->setPos(S->getPos() + Pos);
  Selects.emplace_back(std::move(S));
  selectNext();
}

void ItemSelect::select(std::size_t Idx) {
  if (!Selects.empty()) {
    Selects.at(SelectedIdx)->unselect();
    SelectedIdx = Idx;
    Selects.at(SelectedIdx)->select();
  }
}

void ItemSelect::selectNext() {
  auto NewIdx = SelectedIdx + 1;
  if (NewIdx >= Selects.size()) {
    NewIdx = 0;
  }
  select(NewIdx);
}

void ItemSelect::selectPrev() {
  auto NewIdx = SelectedIdx - 1;
  if (SelectedIdx == 0) {
    NewIdx = Selects.size() - 1;
  }
  select(NewIdx);
}

std::size_t ItemSelect::getSelectedIdx() const { return SelectedIdx; }

Select &ItemSelect::getSelected() { return *Selects.at(SelectedIdx); }

const Select &ItemSelect::getSelected() const {
  return *Selects.at(SelectedIdx);
}

Select &ItemSelect::getSelect(std::size_t Idx) { return *Selects.at(Idx); }

const Select &ItemSelect::getSelect(std::size_t Idx) const {
  return *Selects.at(Idx);
}

void ItemSelect::registerOnSelectCallback(OnSelectCallback OnSelectCb) {
  this->OnSelectCb = std::move(OnSelectCb);
}

void ItemSelect::setPos(cxxg::types::Position P) {
  for (auto &S : Selects) {
    S->setPos(S->getPos() - Pos + P);
  }
  Pos = P;
}

bool ItemSelect::handleInput(int Char) {
  switch (Char) {
  case Controls::CloseWindow.Char:
    return false;
  case Controls::MoveUp.Char:
    selectPrev();
    break;
  case Controls::MoveDown.Char:
    selectNext();
    break;
  case Controls::Interact.Char:
    handleSelect();
    break;
  default:
    break;
  }
  return true;
}

std::string ItemSelect::getInteractMsg() const {
  return Controls::Navigate.getInteractMsg();
}

void ItemSelect::draw(cxxg::Screen &Scr) const {
  for (const auto &S : Selects) {
    S->draw(Scr);
  }
}

void ItemSelect::handleSelect() const {
  if (Selects.empty()) {
    return;
  }
  if (OnSelectCb) {
    OnSelectCb(getSelected());
  }
}

} // namespace rogue::ui