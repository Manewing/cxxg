#ifndef ROGUE_UI_DECORATOR_H
#define ROGUE_UI_DECORATOR_H

#include <cxxg/Types.h>
#include <memory>
#include <rogue/UI/Widget.h>

namespace rogue::ui {

class Decorator : public Widget {
public:
  Decorator(cxxg::types::Position Pos, std::shared_ptr<Widget> Comp);

  const std::shared_ptr<Widget> &getComp() const { return Comp; }
  virtual void setComp(std::shared_ptr<Widget> Comp);

  void setPos(cxxg::types::Position Pos) override;
  bool handleInput(int Char) override;
  std::string_view getInteractMsg() const override;
  void draw(cxxg::Screen &Scr) const override;

protected:
  std::shared_ptr<Widget> Comp;
};

// PreventCloseDecorator
// ResizeDecorator

class ExitHandlerProvider {
public:
  using ExitHandlerCallback = std::function<void()>;

public:
  virtual ~ExitHandlerProvider() = default;

  void registerExitHandler(ExitHandlerCallback EMC);

protected:
  void handleExit();

private:
  ExitHandlerCallback EMC;
};

class MoveDecorator : public Decorator, public ExitHandlerProvider {
public:
  static constexpr auto IconColor = cxxg::types::RgbColor{135, 250, 10};

public:
  explicit MoveDecorator(const std::shared_ptr<Widget> &Comp);

  bool handleInput(int Char) final;
  void draw(cxxg::Screen &Scr) const final;
};

} // namespace rogue::ui

#endif // #ifndef ROGUE_UI_DECORATOR_H