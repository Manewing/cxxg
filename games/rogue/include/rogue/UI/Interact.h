#ifndef ROGUE_UI_INTERACT_UI_H
#define ROGUE_UI_INTERACT_UI_H

#include <entt/entt.hpp>
#include <rogue/UI/Decorator.h>
#include <rogue/UI/Widget.h>
#include <ymir/Types.hpp>

namespace rogue {
class Level;
struct Interaction;
} // namespace rogue

namespace rogue::ui {
class ListSelect;
class Controller;
} // namespace rogue::ui

namespace rogue::ui {

class Interact : public BaseRectDecorator {
public:
  struct InteractionInfo {
    const entt::entity Entity;
    const Interaction &Action;
  };

public:
  Interact(entt::entity SrcEt, ymir::Point2d<int> StartPos, Level &Lvl);

  bool handleInput(int) override;
  std::string getInteractMsg() const override;
  void draw(cxxg::Screen &Scr) const override;

  void destroyCursor();

private:
  void updateElements();
  void updateCursor();
  void handleInteraction();

private:
  Level &Lvl;
  ymir::Point2d<int> StartPos;
  entt::entity SrcEt = entt::null;
  entt::entity CursorEt = entt::null;
  std::vector<entt::entity> InteractablesEts;
  std::shared_ptr<ListSelect> List;
  std::vector<InteractionInfo> Interactions;
};

} // namespace rogue::ui

#endif // #ifndef ROGUE_INTERACT_UI_H