#include <rogue/History.h>
#include <rogue/Game.h>
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

void History::addMessage(const cxxg::Row &Msg) { Messages.push_back(Msg); }

EventHistoryWriter::EventHistoryWriter(History &Hist) : Hist(Hist) {}

void EventHistoryWriter::setEventHub(EventHub *Hub) {
  EventHubConnector::setEventHub(Hub);
  subscribe(*this, &EventHistoryWriter::onEntityAttackEvent);
  subscribe(*this, &EventHistoryWriter::onEntityDiedEvent);
  subscribe(*this, &EventHistoryWriter::onDebugMessageEvent);
}

void EventHistoryWriter::onEntityAttackEvent(const EntityAttackEvent &EAE) {
  if (!EAE.isPlayerAffected()) {
    return;
  }
  Hist.info() << cxxg::types::RgbColor{140, 130, 72} << EAE.Attacker.getName()
              << cxxg::types::Color::NONE << " dealt "
              << cxxg::types::Color::RED << EAE.Damage << " damage"
              << cxxg::types::Color::NONE << " to "
              << cxxg::types::RgbColor{140, 130, 72} << EAE.Target.getName()
              << cxxg::types::Color::NONE;
}

void EventHistoryWriter::onEntityDiedEvent(const EntityDiedEvent &EDE) {
  if (!EDE.IsPlayer) {
    return;
  }
  Hist.warn() << cxxg::types::Color::RED << "You died!";
}

void EventHistoryWriter::onDebugMessageEvent(const DebugMessageEvent &DbgEv) {
  Hist.info() << cxxg::types::Color::GREEN << "Debug: " << DbgEv.Message.str();
}

}