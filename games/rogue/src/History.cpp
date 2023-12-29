#include <rogue/Components/Buffs.h>
#include <rogue/Components/Visual.h>
#include <rogue/Event.h>
#include <rogue/Game.h>
#include <rogue/History.h>
#include <rogue/Systems/DeathSystem.h>

namespace rogue {

HistoryMessageAssembler::HistoryMessageAssembler(History &Hist,
                                                 cxxg::RowAccessor Rw)
    : Hist(Hist), Row(std::move(Rw)) {}

HistoryMessageAssembler::HistoryMessageAssembler(HistoryMessageAssembler &&HMA)
    : Hist(HMA.Hist), Row(std::move(HMA.Row)) {}

HistoryMessageAssembler::~HistoryMessageAssembler() {
  Row.flushBuffer();
  Hist.addMessage(Row.get());
}

History::History(Game &G) : G(G) {}

HistoryMessageAssembler History::info() {
  return HistoryMessageAssembler(*this, G.notify());
}

HistoryMessageAssembler History::warn() {
  return HistoryMessageAssembler(*this, G.notify());
}

void History::addMessage(const cxxg::Row &Msg) {
  if (Messages.size() > 0 && Messages.back().Row == Msg) {
    Messages.back().Count++;
    return;
  }
  Messages.push_back(Message{Msg});
}

EventHistoryWriter::EventHistoryWriter(History &Hist, bool Debug)
    : Hist(Hist), Debug(Debug) {}

void EventHistoryWriter::setEventHub(EventHub *Hub) {
  EventHubConnector::setEventHub(Hub);
  subscribe(*this, &EventHistoryWriter::onEntityAttackEvent);
  subscribe(*this, &EventHistoryWriter::onEntityDiedEvent);
  subscribe(*this, &EventHistoryWriter::onBuffAppliedEvent);
  subscribe(*this, &EventHistoryWriter::onBuffApplyEffectEvent);
  subscribe(*this, &EventHistoryWriter::onBuffExpiredEvent);
  subscribe(*this, &EventHistoryWriter::onPlayerInfoMessageEvent);
  subscribe(*this, &EventHistoryWriter::onWarningMessageEvent);
  subscribe(*this, &EventHistoryWriter::onErrorMessageEvent);
  subscribe(*this, &EventHistoryWriter::onDebugMessageEvent);
}

void EventHistoryWriter::onEntityAttackEvent(const EntityAttackEvent &EAE) {
  if ((!EAE.isPlayerAffected() && !Debug) || !EAE.Registry) {
    return;
  }
  const auto *AttackerNC = EAE.Registry->try_get<NameComp>(EAE.Attacker);
  const auto *TargetNC = EAE.Registry->try_get<NameComp>(EAE.Target);
  if (!AttackerNC || !TargetNC) {
    return;
  }
  if (EAE.Damage) {
    Hist.info() << cxxg::types::RgbColor{140, 130, 72} << AttackerNC->Name
                << cxxg::types::Color::NONE << " dealt "
                << cxxg::types::Color::RED << *EAE.Damage << " damage"
                << cxxg::types::Color::NONE << " to "
                << cxxg::types::RgbColor{140, 130, 72} << TargetNC->Name
                << cxxg::types::Color::NONE;
    return;
  }
  Hist.info() << cxxg::types::RgbColor{140, 130, 72} << TargetNC->Name
              << cxxg::types::Color::NONE << " blocked attack of "
              << cxxg::types::RgbColor{140, 130, 72} << AttackerNC->Name
              << cxxg::types::Color::NONE;
}

void EventHistoryWriter::onEntityDiedEvent(const EntityDiedEvent &EDE) {
  if ((!EDE.isPlayerAffected() && !Debug) || !EDE.Registry) {
    return;
  }
  Hist.warn() << cxxg::types::Color::RED << "You died!";
}

void EventHistoryWriter::onBuffAppliedEvent(const BuffAppliedEvent &BAE) {
  if ((!BAE.isPlayerAffected() && !Debug) || !BAE.Buff || !BAE.Registry) {
    return;
  }
  const auto *SrcNC = BAE.Registry->try_get<NameComp>(BAE.SrcEt);
  const auto *TargetNC = BAE.Registry->try_get<NameComp>(BAE.TargetEt);
  if (!SrcNC || !TargetNC) {
    return;
  }
  cxxg::types::TermColor BuffColor = cxxg::types::Color::GREEN;
  if (BAE.IsCombat) {
    BuffColor = cxxg::types::Color::RED;
  }
  Hist.info() << cxxg::types::RgbColor{140, 130, 72} << SrcNC->Name
              << cxxg::types::Color::NONE << " applied " << BuffColor
              << BAE.Buff->getName() << cxxg::types::Color::NONE << " to "
              << cxxg::types::RgbColor{140, 130, 72} << TargetNC->Name
              << cxxg::types::Color::NONE;
}

void EventHistoryWriter::onBuffApplyEffectEvent(
    const BuffApplyEffectEvent &BAEE) {
  if ((!BAEE.isPlayerAffected() && !Debug) || !BAEE.Buff) {
    return;
  }
  const auto *NC = BAEE.Registry->try_get<NameComp>(BAEE.Entity);
  if (!NC) {
    return;
  }
  cxxg::types::TermColor BuffColor = cxxg::types::Color::GREEN;
  if (BAEE.IsReduce) {
    BuffColor = cxxg::types::Color::RED;
  }
  Hist.info() << BuffColor << BAEE.Buff->getApplyDesc()
              << cxxg::types::Color::NONE << " on "
              << cxxg::types::RgbColor{140, 130, 72} << NC->Name;
}

void EventHistoryWriter::onBuffExpiredEvent(const BuffExpiredEvent &BEE) {
  if ((!BEE.isPlayerAffected() && !Debug) || !BEE.Buff) {
    return;
  }
  const auto *NC = BEE.Registry->try_get<NameComp>(BEE.Entity);
  if (!NC) {
    return;
  }
  Hist.info() << "Buff " << cxxg::types::RgbColor{201, 198, 139}
              << BEE.Buff->getName() << cxxg::types::Color::NONE
              << " expired on " << cxxg::types::RgbColor{140, 130, 72}
              << NC->Name << cxxg::types::Color::NONE;
}

void EventHistoryWriter::onPlayerInfoMessageEvent(
    const PlayerInfoMessageEvent &PIME) {
  Hist.info() << cxxg::types::Color::NONE << PIME.Message.str();
}

void EventHistoryWriter::onWarningMessageEvent(const WarningMessageEvent &WME) {
  Hist.warn() << cxxg::types::Color::YELLOW << "Warning: " << WME.Message.str();
}

void EventHistoryWriter::onErrorMessageEvent(const ErrorMessageEvent &ErrEv) {
  Hist.warn() << cxxg::types::Color::RED << "Error: " << ErrEv.Message.str();
}

void EventHistoryWriter::onDebugMessageEvent(const DebugMessageEvent &DbgEv) {
  Hist.info() << cxxg::types::Color::GREEN << "Debug: " << DbgEv.Message.str();
}

} // namespace rogue