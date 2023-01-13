#include <cxxg/Screen.h>
#include <rogue/UIListSelect.h>

namespace rogue {

UIListSelect::UIListSelect(std::string Hdr, unsigned Width, unsigned MaxRows)
    : Header(std::move(Hdr)), Width(Width), MaxRows(MaxRows) {}

void UIListSelect::setElements(const std::vector<std::string> &Elements) {
  this->Elements = Elements;
  SelectedElemIdx = 0;
}

void UIListSelect::selectElement(std::size_t ElemIdx) {
  SelectedElemIdx = ElemIdx;
  if (SelectedElemIdx >= Elements.size()) {
    SelectedElemIdx = 0;
  }
}

std::size_t UIListSelect::getSelectedElement() const { return SelectedElemIdx; }

void UIListSelect::selectNext() { selectElement(SelectedElemIdx + 1); }

void UIListSelect::selectPrev() {
  if (SelectedElemIdx == 0) {
    selectElement(Elements.size() - 1);
    return;
  }
  selectElement(SelectedElemIdx - 1);
}

void UIListSelect::draw(cxxg::Screen &Scr) const {
  const int PosX = 2, PosY = 2;

  // Draw frame
  if (Header.empty()) {
    drawFrameHLine(Scr, {PosX, PosY}, Width);
  } else {
    drawFrameHeader(Scr, {PosX, PosY}, Header, Width);
  }
  for (unsigned Row = 0; Row < MaxRows; Row++) {
    drawFrameVLine(Scr, {PosX, PosY + static_cast<int>(Row) + 1}, Width);
  }
  drawFrameHLine(Scr, {PosX, PosY + static_cast<int>(MaxRows) + 1}, Width);

  // Draw elements
  for (unsigned ElemIdx = 0; ElemIdx < MaxRows && ElemIdx < Elements.size();
       ElemIdx++) {
    const auto &Element = Elements.at(ElemIdx);
    drawFrameElement(Scr, Element, {PosX, PosY + static_cast<int>(ElemIdx) + 1},
                     Width, ElemIdx == SelectedElemIdx);
  }
}

void UIListSelect::drawFrameHeader(cxxg::Screen &Scr, cxxg::types::Position Pos,
                                   std::string_view Header, unsigned Width) {
  drawFrameHLine(Scr, Pos, Width);
  unsigned HdrOffset = (Width - Header.size()) / 2 - 1;
  Scr[Pos.Y][Pos.X + HdrOffset] << "[" << Header << "]";
}

void UIListSelect::drawFrameHLine(cxxg::Screen &Scr, cxxg::types::Position Pos,
                                  unsigned Width) {
  Scr[Pos.Y][Pos.X] << "+" << std::string(Width - 2, '-') << "+";
}

void UIListSelect::drawFrameVLine(cxxg::Screen &Scr, cxxg::types::Position Pos,
                                  unsigned Width) {
  Scr[Pos.Y][Pos.X] << "|" << std::string(Width - 2, ' ') << "|";
}

void UIListSelect::drawFrameElement(cxxg::Screen &Scr, std::string_view Element,
                                    cxxg::types::Position Pos, unsigned Width,
                                    bool IsSelected) {
  if (IsSelected) {
    Scr[Pos.Y][Pos.X + 1] << ">";
  }
  (void)Width;
  Scr[Pos.Y][Pos.X + 2] << Element;
}

} // namespace rogue