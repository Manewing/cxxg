#ifndef ROGUE_UI_ITEM_SELECT_H
#define ROGUE_UI_ITEM_SELECT_H

#include <cxxg/Types.h>
#include <memory>
#include <rogue/UI/Widget.h>
#include <string>
#include <vector>

namespace rogue::ui {

class LabeledSelect : public BaseRect {
public:
  LabeledSelect(std::string Label, std::string Value, cxxg::types::Position Pos,
                unsigned Width);

  const std::string &getLabel() const;
  void setValue(std::string NewValue);
  const std::string &getValue() const;

  void unselect();
  void select();
  bool isSlected() const;

  bool handleInput(int Char) override;
  std::string_view getInteractMsg() const override;
  void draw(cxxg::Screen &Scr) const override;

private:
  bool IsSelected = false;
  std::string Label;
  std::string Value;
};

class ItemSelect : public Widget {
public:
  void addSelect(std::shared_ptr<LabeledSelect> Select);

  void selectNext();
  void selectPrev();

  std::size_t getSelectedIdx() const;
  LabeledSelect &getSelected();
  LabeledSelect &getSelect(std::size_t Idx);

  bool handleInput(int Char) override;
  std::string_view getInteractMsg() const override;
  void draw(cxxg::Screen &Scr) const override;

protected:
  std::size_t SelectedIdx = 0;
  std::vector<std::shared_ptr<LabeledSelect>> Selects;
};

} // namespace rogue::ui

#endif //  #ifndef ROGUE_UI_ITEM_SELECT_H
