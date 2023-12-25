#ifndef ROGUE_UI_LIST_SELECT_H
#define ROGUE_UI_LIST_SELECT_H

#include <cxxg/Types.h>
#include <rogue/UI/Widget.h>
#include <string>
#include <vector>

namespace rogue::ui {

/// Creates a new list select widget
///
/// Draws a sequence of elements in a list, with one element selected
///
///     "         "
///     " >Elem 1 "
///     "  Elem 2 "
///     "         "
///
class ListSelect : public BaseRect {
public:
  struct Element {
    std::string Text;
    cxxg::types::TermColor Color = cxxg::types::Color::NONE;
  };

public:
  static void drawFrameElement(cxxg::Screen &Scr, const Element &Elem,
                               cxxg::types::Position Pos, unsigned Width,
                               bool IsSelected);

public:
  ListSelect(cxxg::types::Position Pos, cxxg::types::Size Size,
             cxxg::types::Size Padding = {1, 1});

  void setElements(const std::vector<Element> &Elements);
  void selectElement(std::size_t ElemIdx);
  std::size_t getSelectedElement() const;
  void selectNext();
  void selectPrev();

  bool handleInput(int Char) override;
  std::string getInteractMsg() const override;
  void draw(cxxg::Screen &Scr) const override;

private:
  cxxg::types::Size Padding;
  std::size_t SelectedElemIdx = 0;
  std::vector<Element> Elements;
};

} // namespace rogue::ui

#endif // #ifndef ROGUE_UI_LIST_SELECT_H