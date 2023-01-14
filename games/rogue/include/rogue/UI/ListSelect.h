#ifndef ROGUE_UI_LIST_SELECT_H
#define ROGUE_UI_LIST_SELECT_H

#include <cxxg/Types.h>
#include <string>
#include <vector>

namespace cxxg {
class Screen;
}

namespace rogue::ui {

class ListSelect {
public:
  ListSelect(std::string Header, unsigned Width, unsigned MaxRows);

  void setElements(const std::vector<std::string> &Elements);
  void selectElement(std::size_t ElemIdx);
  std::size_t getSelectedElement() const;
  void selectNext();
  void selectPrev();

  void draw(cxxg::Screen &Scr) const;

  static void drawFrameHeader(cxxg::Screen &Scr, cxxg::types::Position Pos,
                              std::string_view Header, unsigned Width);
  static void drawFrameHLine(cxxg::Screen &Scr, cxxg::types::Position Pos,
                             unsigned Width);
  static void drawFrameVLine(cxxg::Screen &Scr, cxxg::types::Position Pos,
                             unsigned Width);
  static void drawFrameElement(cxxg::Screen &Scr, std::string_view Element,
                               cxxg::types::Position Pos, unsigned Width,
                               bool IsSelected);

  std::string Header;
  unsigned Width;
  unsigned MaxRows;

  std::size_t SelectedElemIdx = 0;
  std::vector<std::string> Elements;
};

} // namespace rogue::ui

#endif // #ifndef ROGUE_UI_LIST_SELECT_H