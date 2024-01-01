#ifndef ROGUE_RENDER_EVENT_COLLECTOR_H
#define ROGUE_RENDER_EVENT_COLLECTOR_H

#include <functional>
#include <rogue/EventHub.h>

namespace rogue {
struct EntityAttackEvent;
struct DetectTargetEvent;
struct LostTargetEvent;
struct EffectDelayEvent;
struct BuffAppliedEvent;
struct BuffExpiredEvent;
struct BuffApplyEffectEvent;
class Renderer;
} // namespace rogue

namespace rogue {

class RenderEventCollector : public EventHubConnector {
public:
  void setEventHub(EventHub *EH) override;
  void onEntityAttackEvent(const EntityAttackEvent &E);
  void onDetectTargetEvent(const DetectTargetEvent &E);
  void onLostTargetEvent(const LostTargetEvent &E);
  void onEffectDelayEvent(const EffectDelayEvent &E);
  void onBuffAppliedEvent(const BuffAppliedEvent &E);
  void onBuffExpiredEvent(const BuffExpiredEvent &E);
  void onBuffApplyEffectEvent(const BuffApplyEffectEvent &E);
  void apply(Renderer &R);
  void clear();
  bool hasEvents() const;

private:
  std::vector<std::function<void(Renderer &)>> RenderFns;
};

} // namespace rogue

#endif // #ifndef ROGUE_RENDER_EVENT_COLLECTOR_H