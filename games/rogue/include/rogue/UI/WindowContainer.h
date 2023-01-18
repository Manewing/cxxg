#ifndef ROGUE_UI_WINDOW_CONTAINER_H
#define ROGUE_UI_WINDOW_CONTAINER_H

#include <memory>
#include <rogue/UI/Decorator.h>
#include <rogue/UI/Widget.h>
#include <vector>

namespace rogue::ui {

class WindowContainer : public Widget {
public:
  static constexpr int KEY_MOVE = 'm';
  static constexpr int KEY_NEXT_WINDOW = 'C';
  static constexpr int KEY_PREV_WINDOW = 'V';
  static constexpr int KEY_AUTO_LAYOUT = 'X';

  static constexpr auto ActiveColor = cxxg::types::RgbColor{90, 130, 175};

public:
  using Widget::Widget;

  bool handleInput(int Char) override;
  std::string_view getInteractMsg() const override;
  void draw(cxxg::Screen &Scr) const override;

  bool hasActiveWindow() const;

  Widget &getActiveWindow();

  const Widget &getActiveWindow() const;

  std::shared_ptr<Widget> &getActiveWindowPtr();

  /// @brief Closes the active window if there is one
  /// @return True if there is still is a window left, otherwise false
  bool closeActiveWindow();

  void addWindow(std::shared_ptr<Widget> Window);

  void selectWindow(std::size_t Idx);
  void selectNextWindow();
  void selectPrevWindow();

  void autoLayoutWindows(cxxg::types::Position StartPos,
                         cxxg::types::Size Size);

protected:
  bool exitMoveActiveWindow();
  bool enterMoveActiveWindow();
  void switchMoveActiveWindow(bool IsMoving);

private:
  std::size_t FocusIdx = 0;
  std::shared_ptr<MoveDecorator> MoveDeco = nullptr;
  std::vector<std::shared_ptr<Widget>> Windows;
};

} // namespace rogue::ui

#endif // #ifndef ROGUE_UI_WINDOW_CONTAINER_H