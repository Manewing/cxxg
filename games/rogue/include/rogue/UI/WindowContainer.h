#ifndef ROGUE_UI_WINDOW_CONTAINER_H
#define ROGUE_UI_WINDOW_CONTAINER_H

#include <memory>
#include <rogue/UI/Decorator.h>
#include <rogue/UI/Widget.h>
#include <optional>
#include <vector>

namespace rogue::ui {

class WindowContainer : public Widget {
public:
  struct WindowInfo {
    cxxg::types::Position Pos{};
    cxxg::types::Size Size{};
    unsigned long Area = 0;
    Widget *Wdw = nullptr;

    static std::optional<WindowInfo> getWindowInfo(Widget *Wdw);
  };

public:
  static constexpr int KEY_MOVE = 'm';
  static constexpr int KEY_NEXT_WINDOW = 'C';
  static constexpr int KEY_PREV_WINDOW = 'V';
  static constexpr int KEY_AUTO_LAYOUT = 'X';

  static constexpr auto ActiveColor = cxxg::types::RgbColor{90, 130, 175};

public:
  WindowContainer(cxxg::types::Position Pos, cxxg::types::Size Size);

  bool handleInput(int Char) override;
  std::string getInteractMsg() const override;
  void draw(cxxg::Screen &Scr) const override;

  bool hasActiveWindow() const;

  template <typename T> Widget *getWindowOfType() const {
    for (const auto &Wdw : Windows) {
      if (dynamic_cast<T *>(Wdw.get())) {
        return Wdw.get();
      }
    }
    return nullptr;
  }

  template <typename T> bool hasWindowOfType() const {
    return getWindowOfType<T>() != nullptr;
  }

  Widget &getActiveWindow();

  const Widget &getActiveWindow() const;

  std::shared_ptr<Widget> &getActiveWindowPtr();

  /// @brief Closes the active window if there is one
  /// @return True if there is still is a window left, otherwise false
  bool closeWindow(std::size_t Idx);
  bool closeWindow(Widget *Wdw);
  bool closeActiveWindow();

  void addWindow(std::shared_ptr<Widget> Window);

  template <typename T, typename... Args> void addWindow(Args &&...Arg) {
    addWindow(std::make_shared<T>(Arg...));
  }

  void selectWindow(std::size_t Idx);
  void selectNextWindow();
  void selectPrevWindow();

  void autoLayoutWindows();
  void autoLayoutWindows(cxxg::types::Position StartPos,
                         cxxg::types::Size Size);

protected:
  bool exitMoveActiveWindow();
  bool enterMoveActiveWindow();
  void switchMoveActiveWindow(bool IsMoving);

private:
  cxxg::types::Size Size;
  std::size_t FocusIdx = 0;
  std::shared_ptr<MoveDecorator> MoveDeco = nullptr;
  std::vector<std::shared_ptr<Widget>> Windows;
};

} // namespace rogue::ui

#endif // #ifndef ROGUE_UI_WINDOW_CONTAINER_H