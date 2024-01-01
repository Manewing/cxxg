#ifndef ROGUE_UI_ITEM_SELECT_H
#define ROGUE_UI_ITEM_SELECT_H

#include <cxxg/Types.h>
#include <functional>
#include <memory>
#include <rogue/UI/Widget.h>
#include <string>
#include <vector>

namespace rogue::ui {

class Select : public BaseRect {
public:
  Select(std::string Value, cxxg::types::Position Pos, unsigned Width,
         cxxg::types::TermColor ValueColor = cxxg::types::Color::NONE);

  void setValue(std::string NewValue);
  const std::string &getValue() const;

  const cxxg::types::TermColor &getValueColor() const;
  void setValueColor(cxxg::types::TermColor ValueColor);

  void unselect();
  void select();
  bool isSlected() const;

  bool handleInput(int Char) override;
  std::string getInteractMsg() const override;
  void draw(cxxg::Screen &Scr) const override;

protected:
  bool IsSelected = false;
  std::string Value;
  cxxg::types::TermColor ValueColor = cxxg::types::Color::NONE;
};

class LabeledSelect : public Select {
public:
  LabeledSelect(std::string Label, std::string Value, cxxg::types::Position Pos,
                unsigned Width,
                cxxg::types::TermColor ValueColor = cxxg::types::Color::NONE,
                cxxg::types::TermColor LabelColor = cxxg::types::Color::NONE);

  const std::string &getLabel() const;
  const cxxg::types::TermColor &getLabelColor() const;
  std::string getInteractMsg() const override;
  void draw(cxxg::Screen &Scr) const override;

private:
  std::string Label;
  cxxg::types::TermColor LabelColor = cxxg::types::Color::NONE;
};

class ItemSelect : public Widget {
public:
  using OnSelectCallback = std::function<void(const Select &)>;

public:
  using Widget::Widget;

  void addSelect(std::shared_ptr<Select> Select);

  template <typename T, typename... Args> void addSelect(Args &&...Arg) {
    addSelect(std::make_shared<T>(Arg...));
  }

  void select(std::size_t Idx);
  void selectNext();
  void selectPrev();

  std::size_t getSelectedIdx() const;
  Select &getSelected();
  const Select &getSelected() const;
  Select &getSelect(std::size_t Idx);
  const Select &getSelect(std::size_t Idx) const;

  void registerOnSelectCallback(OnSelectCallback OnSelectCb);

  void setPos(cxxg::types::Position Pos) override;
  bool handleInput(int Char) override;
  std::string getInteractMsg() const override;
  void draw(cxxg::Screen &Scr) const override;

  void handleSelect() const;

protected:
  std::size_t SelectedIdx = 0;
  std::vector<std::shared_ptr<Select>> Selects;
  OnSelectCallback OnSelectCb;
};

} // namespace rogue::ui

#endif //  #ifndef ROGUE_UI_ITEM_SELECT_H
