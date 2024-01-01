#include <rogue/Components/Transform.h>
#include <rogue/Event.h>
#include <rogue/RenderEventCollector.h>
#include <rogue/Renderer.h>

namespace rogue {
void RenderEventCollector::setEventHub(EventHub *EH) {
  EventHubConnector::setEventHub(EH);
  EH->subscribe(*this, &RenderEventCollector::onEntityAttackEvent);
  EH->subscribe(*this, &RenderEventCollector::onDetectTargetEvent);
  EH->subscribe(*this, &RenderEventCollector::onLostTargetEvent);
  EH->subscribe(*this, &RenderEventCollector::onEffectDelayEvent);
  EH->subscribe(*this, &RenderEventCollector::onBuffAppliedEvent);
  EH->subscribe(*this, &RenderEventCollector::onBuffExpiredEvent);
  EH->subscribe(*this, &RenderEventCollector::onBuffApplyEffectEvent);
}

void RenderEventCollector::onEntityAttackEvent(const EntityAttackEvent &E) {
  auto *PC = E.Registry->try_get<PositionComp>(E.Target);
  if (!PC) {
    return;
  }
  auto const AtPos = PC->Pos;
  RenderFns.push_back([AtPos](Renderer &R) {
    R.renderEffect(
        cxxg::types::ColoredChar{'*', cxxg::types::RgbColor{155, 20, 20}},
        AtPos);
  });
}

void RenderEventCollector::onDetectTargetEvent(const DetectTargetEvent &E) {
  auto *PC = E.Registry->try_get<PositionComp>(E.Entity);
  if (!PC) {
    return;
  }
  auto const AtPos = PC->Pos;
  RenderFns.push_back([AtPos](Renderer &R) {
    R.renderEffect(
        cxxg::types::ColoredChar{'!', cxxg::types::RgbColor{173, 161, 130}},
        AtPos);
  });
}

void RenderEventCollector::onLostTargetEvent(const LostTargetEvent &E) {
  auto *PC = E.Registry->try_get<PositionComp>(E.Entity);
  if (!PC) {
    return;
  }
  auto const AtPos = PC->Pos;
  RenderFns.push_back([AtPos](Renderer &R) {
    R.renderEffect(
        cxxg::types::ColoredChar{'?', cxxg::types::RgbColor{56, 55, 89}},
        AtPos);
  });
}

void RenderEventCollector::onEffectDelayEvent(const EffectDelayEvent &) {
  RenderFns.push_back([](Renderer &) {});
}

void RenderEventCollector::onBuffAppliedEvent(const BuffAppliedEvent &E) {
  auto *PC = E.Registry->try_get<PositionComp>(E.TargetEt);
  if (!PC) {
    return;
  }
  auto const AtPos = PC->Pos;
  RenderFns.push_back([AtPos, IsCombat = E.IsCombat](Renderer &R) {
    if (IsCombat) {
      R.renderEffect(
          cxxg::types::ColoredChar{'v', cxxg::types::RgbColor{100, 10, 10}},
          AtPos);
    } else {
      R.renderEffect(
          cxxg::types::ColoredChar{'^', cxxg::types::RgbColor{10, 100, 10}},
          AtPos);
    }
  });
}

void RenderEventCollector::onBuffExpiredEvent(const BuffExpiredEvent &E) {
  auto *PC = E.Registry->try_get<PositionComp>(E.Entity);
  if (!PC) {
    return;
  }
  auto const AtPos = PC->Pos;
  RenderFns.push_back([AtPos](Renderer &R) {
    R.renderEffect(
        cxxg::types::ColoredChar{';', cxxg::types::RgbColor{100, 100, 100}},
        AtPos);
  });
}

void RenderEventCollector::onBuffApplyEffectEvent(
    const BuffApplyEffectEvent &E) {
  auto *PC = E.Registry->try_get<PositionComp>(E.Entity);
  if (!PC) {
    return;
  }
  auto const AtPos = PC->Pos;
  RenderFns.push_back([AtPos, IsReduce = E.IsReduce](Renderer &R) {
    if (IsReduce) {
      R.renderEffect(
          cxxg::types::ColoredChar{'-', cxxg::types::RgbColor{100, 10, 10}},
          AtPos);
    } else {
      R.renderEffect(
          cxxg::types::ColoredChar{'+', cxxg::types::RgbColor{10, 100, 10}},
          AtPos);
    }
  });
}

void RenderEventCollector::apply(Renderer &R) {
  for (auto &Fn : RenderFns) {
    Fn(R);
  }
}

void RenderEventCollector::clear() { RenderFns.clear(); }

bool RenderEventCollector::hasEvents() const { return !RenderFns.empty(); }

} // namespace rogue