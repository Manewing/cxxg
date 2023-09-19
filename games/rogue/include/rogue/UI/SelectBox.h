#ifndef ROGUE_UI_SELECT_BOX_H
#define ROGUE_UI_SELECT_BOX_H

#include <rogue/UI/ItemSelect.h>
#include <rogue/UI/Widget.h>
#include <string>

namespace rogue::ui {

class SelectBox : public BaseRect {
public:
  struct Option {
    char ShortCut = 'e';
    std::string Text;

    std::string getValue() const;
  };

public:
  static cxxg::types::Size getSize(const std::vector<Option> &Options);

public:
  SelectBox(cxxg::types::Position Pos, const std::vector<Option> &Options);

  void registerOnSelectCallback(ItemSelect::OnSelectCallback OnSelectCb);

  void setPos(cxxg::types::Position P) override;

  bool handleInput(int Char) override;

  std::string getInteractMsg() const override;

  void draw(cxxg::Screen &Scr) const override;

protected:
  std::shared_ptr<ItemSelect> ItSel;
  std::shared_ptr<Widget> Decorated;
  std::vector<Option> Options;
};

} // namespace rogue::ui

#endif // #ifndef ROGUE_UI_SELECT_BOX_H