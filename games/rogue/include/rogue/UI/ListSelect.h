#ifndef ROGUE_UI_LIST_SELECT_H
#define ROGUE_UI_LIST_SELECT_H

#include <cxxg/Types.h>
#include <rogue/UI/Widget.h>
#include <string>
#include <vector>

namespace rogue::ui {

class ListSelect : public BaseRect {
public:
  static void drawFrameElement(cxxg::Screen &Scr, std::string_view Element,
                               cxxg::types::Position Pos, unsigned Width,
                               bool IsSelected);

public:
  using BaseRect::BaseRect;

  void setElements(const std::vector<std::string> &Elements);
  void selectElement(std::size_t ElemIdx);
  std::size_t getSelectedElement() const;
  void selectNext();
  void selectPrev();

  bool handleInput(int Char) override;
  std::string_view getInteractMsg() const override;
  void draw(cxxg::Screen &Scr) const override;

private:
  std::size_t SelectedElemIdx = 0;
  std::vector<std::string> Elements;
};

} // namespace rogue::ui

#endif // #ifndef ROGUE_UI_LIST_SELECT_H