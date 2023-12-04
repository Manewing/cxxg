#ifndef ROGUE_UI_TEXT_BOX_H
#define ROGUE_UI_TEXT_BOX_H

#include <rogue/UI/Widget.h>
#include <rogue/UI/WordWrap.h>

namespace rogue::ui {

class TextBox : public BaseRect {
public:
  TextBox(cxxg::types::Position Pos, cxxg::types::Size Size,
          const std::string &Text, cxxg::types::Size Padding = {2, 1});

  void setText(const std::string &Text);

  bool handleInput(int Char) final;

  std::string getInteractMsg() const final;

  void draw(cxxg::Screen &Scr) const override;

protected:
  WordWrap Wrap;
  cxxg::types::Size Padding;
  std::size_t ScrollIdx = 0;
};

} // namespace rogue::ui

#endif // #ifndef ROGUE_UI_TEXT_BOX_H