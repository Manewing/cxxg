#include <cxxg/Screen.h>
#include <cxxg/Utils.h>
#include <rogue/UI/ListSelect.h>

namespace rogue::ui {

void ListSelect::setElements(const std::vector<std::string> &Elements) {
  this->Elements = Elements;
  SelectedElemIdx = 0;
}

void ListSelect::selectElement(std::size_t ElemIdx) {
  SelectedElemIdx = ElemIdx;
  if (SelectedElemIdx >= Elements.size()) {
    SelectedElemIdx = 0;
  }
}

std::size_t ListSelect::getSelectedElement() const { return SelectedElemIdx; }

void ListSelect::selectNext() { selectElement(SelectedElemIdx + 1); }

void ListSelect::selectPrev() {
  if (SelectedElemIdx == 0) {
    selectElement(Elements.size() - 1);
    return;
  }
  selectElement(SelectedElemIdx - 1);
}

bool ListSelect::handleInput(int Char) {
  switch (Char) {
  case cxxg::utils::KEY_DOWN:
    selectNext();
    break;
  case cxxg::utils::KEY_UP:
    selectPrev();
    break;
  default:
    break;
  }
  return true;
}

std::string_view ListSelect::getInteractMsg() const { return ""; }

void ListSelect::draw(cxxg::Screen &Scr) const {
  // Fill rect
  BaseRect::draw(Scr);

  // Draw elements
  for (unsigned ElemIdx = 0; ElemIdx < Size.Y && ElemIdx < Elements.size();
       ElemIdx++) {
    const auto &Element = Elements.at(ElemIdx);
    drawFrameElement(Scr, Element, {Pos.X, Pos.Y + static_cast<int>(ElemIdx) + 1},
                     Size.X, ElemIdx == SelectedElemIdx);
  }
}

void ListSelect::drawFrameElement(cxxg::Screen &Scr, std::string_view Element,
                                  cxxg::types::Position Pos, unsigned Width,
                                  bool IsSelected) {
  if (IsSelected) {
    Scr[Pos.Y][Pos.X + 1] << ">";
  }
  (void)Width;
  Scr[Pos.Y][Pos.X + 2] << Element;
}

} // namespace rogue::ui