#include <cxxg/Screen.h>
#include <cxxg/Utils.h>
#include <rogue/UI/Controls.h>
#include <rogue/UI/ListSelect.h>

namespace rogue::ui {

ListSelect::ListSelect(cxxg::types::Position Pos, cxxg::types::Size Size,
                       cxxg::types::Size Padding)
    : BaseRect(Pos, Size), Padding(Padding) {}

void ListSelect::setElements(const std::vector<Element> &Elements) {
  this->Elements = Elements;
  SelectedElemIdx = 0;
}

const std::vector<ListSelect::Element> &ListSelect::getElements() const {
  return Elements;
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
  case Controls::CloseWindow.Char:
    return false;
  case Controls::MoveDown.Char:
    selectNext();
    break;
  case Controls::MoveUp.Char:
    selectPrev();
    break;
  default:
    break;
  }
  return true;
}

std::string ListSelect::getInteractMsg() const {
  return Controls::Navigate.getInteractMsg();
}

void ListSelect::draw(cxxg::Screen &Scr) const {
  // Fill rect
  BaseRect::draw(Scr);

  // Compute available size for elements (number of rows)
  const auto AvailableSize = cxxg::types::Size::clamp(getSize() - Padding * 2);

  // Compute the first and last element to draw in the list based on the
  // selected element and the available size
  unsigned FirstElemIdx = 0, LastElemIdx = 0;
  if (SelectedElemIdx >= AvailableSize.Y) {
    FirstElemIdx = SelectedElemIdx - AvailableSize.Y + 1;
  }
  LastElemIdx = FirstElemIdx + AvailableSize.Y;

  // Draw elements
  unsigned ElemNum = 0;
  for (unsigned ElemIdx = FirstElemIdx;
       ElemIdx < LastElemIdx && ElemIdx < Elements.size(); ElemIdx++) {
    const auto &Element = Elements.at(ElemIdx);
    drawFrameElement(Scr, Element,
                     {static_cast<int>(Pos.X + Padding.X),
                      static_cast<int>(Pos.Y + ElemNum++ + Padding.Y)},
                     AvailableSize.X, ElemIdx == SelectedElemIdx);
  }
}

void ListSelect::drawFrameElement(cxxg::Screen &Scr, const Element &Elem,
                                  cxxg::types::Position Pos, unsigned Width,
                                  bool IsSelected) {
  if (IsSelected) {
    Scr[Pos.Y][Pos.X] << ">";
  }
  Scr[Pos.Y][Pos.X + 1] << Elem.Color << cxxg::RWidth(Width - 1) << Elem.Text
                        << cxxg::types::Color::NONE;
}

} // namespace rogue::ui