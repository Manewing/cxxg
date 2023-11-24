#ifndef ROGUE_UI_COMMAND_LINE_H
#define ROGUE_UI_COMMAND_LINE_H

#include <cxxg/Types.h>
#include <rogue/EventHub.h>
#include <rogue/UI/Decorator.h>
#include <string>

namespace rogue {
class Level;
} // namespace rogue

namespace rogue::ui {
class Controller;
} // namespace rogue::ui

namespace rogue::ui {

class TextLineEdit : public BaseRect {
public:
  TextLineEdit() = delete;
  TextLineEdit(cxxg::types::Position Pos, unsigned Width);
  void draw(cxxg::Screen &Scr) const override;
  bool handleInput(int Char) override;
  void setText(const std::string &Text);
  const std::string &getText() const;

private:
  std::string Text;
	std::size_t CursorPos = 0;
};

class CommandLineController : public BaseRectDecorator,
                              public EventHubConnector {
public:
  CommandLineController() = delete;
  CommandLineController(Controller &Ctrl, Level &Lvl);
  bool handleInput(int Char) override;
  std::string getInteractMsg() const override;
  void draw(cxxg::Screen &Scr) const override;

  bool handleCommandLine(const std::string &Cmd);

private:
  Controller &Ctrl;
  Level &Lvl;
  std::shared_ptr<TextLineEdit> CmdLine;
};

} // namespace rogue::ui

#endif // #define ROGUE_UI_COMMAND_LINE_H