#ifndef ROGUE_TARGET_UI_H
#define ROGUE_TARGET_UI_H

#include <entt/entt.hpp>
#include <rogue/UI/Decorator.h>
#include <rogue/UI/Widget.h>
#include <ymir/Types.hpp>

namespace rogue {
class Level;
} // namespace rogue

namespace rogue::ui {
class Controller;
} // namespace rogue::ui

namespace rogue::ui {

class TargetInfo : public BaseRectDecorator {
public:
  TargetInfo(Controller &Ctrl, entt::entity TargetEt, Level &Lvl);
  void draw(cxxg::Screen &Scr) const override;

private:
  Controller &Ctrl;
  entt::entity TargetEt;
  Level &Lvl;
};

class TargetUI : public Widget {
public:
  /// Callback called when a target has been selected
  /// \param TargetEt The entity that was selected or null
  /// \param TargetPos The position that was selected
  using SelectTargetCb = std::function<void(entt::entity, ymir::Point2d<int>)>;

public:
  TargetUI(Controller &Ctrl, ymir::Point2d<int> StartPos, std::optional<unsigned> Range,
           Level &Lvl, const SelectTargetCb &Callback);
  bool handleInput(int) override;
  std::string getInteractMsg() const override;
  void draw(cxxg::Screen &Scr) const override;

  entt::entity getTarget() const { return TargetEt; }
  ymir::Point2d<int> getCursorPos() const;

  void destroyCursor();
  void showInfoForTarget(entt::entity TargetEt);

private:
  Controller &Ctrl;
  ymir::Point2d<int> StartPos;
  std::optional<unsigned> Range;
  Level &Lvl;
  SelectTargetCb SelectCb;
  entt::entity CursorEt = entt::null;
  entt::entity TargetEt = entt::null;
};

} // namespace rogue::ui

#endif // #ifndef ROGUE_TARGET_UI_H