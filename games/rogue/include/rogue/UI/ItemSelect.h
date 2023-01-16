#ifndef ROGUE_UI_ITEM_SELECT_H
#define ROGUE_UI_ITEM_SELECT_H

#include <cxxg/Types.h>
#include <memory>
#include <rogue/UI/Widget.h>
#include <string>
#include <vector>

namespace rogue::ui {

class Select : public BaseRect {
public:
  Select(std::string Value, cxxg::types::Position Pos,
                unsigned Width);

  void setValue(std::string NewValue);
  const std::string &getValue() const;

  void unselect();
  void select();
  bool isSlected() const;

  bool handleInput(int Char) override;
  std::string_view getInteractMsg() const override;
  void draw(cxxg::Screen &Scr) const override;

protected:
  bool IsSelected = false;
  std::string Value;
};

class LabeledSelect : public Select {
public:
  LabeledSelect(std::string Label, std::string Value, cxxg::types::Position Pos,
                unsigned Width);

  const std::string &getLabel() const;
  std::string_view getInteractMsg() const override;
  void draw(cxxg::Screen &Scr) const override;

private:
  std::string Label;
};

class ItemSelect : public Widget {
public:
  using Widget::Widget;

  void addSelect(std::shared_ptr<Select> Select);

  template <typename T, typename ... Args>
  void addSelect(Args && ...Arg) {
    addSelect(std::make_shared<T>(Arg...));
  }

  void select(std::size_t Idx);
  void selectNext();
  void selectPrev();

  std::size_t getSelectedIdx() const;
  Select &getSelected();
  Select &getSelect(std::size_t Idx);

  void setPos(cxxg::types::Position Pos) override;
  bool handleInput(int Char) override;
  std::string_view getInteractMsg() const override;
  void draw(cxxg::Screen &Scr) const override;

protected:
  std::size_t SelectedIdx = 0;
  std::vector<std::shared_ptr<Select>> Selects;
};

} // namespace rogue::ui

#endif //  #ifndef ROGUE_UI_ITEM_SELECT_H
