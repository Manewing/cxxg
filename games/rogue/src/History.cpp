#include "History.h"
#include "Game.h"

HistoryMessageAssembler::HistoryMessageAssembler(History &Hist,
                                                 cxxg::RowAccessor Row)
    : Hist(Hist), Row(Row) {}

HistoryMessageAssembler::~HistoryMessageAssembler() {
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
}

void EventHistoryWriter::onEntityAttackEvent(const EntityAttackEvent &EAE) {
  if (!EAE.isPlayerAffected()) {
    return;
  }
  Hist.info() << EAE.Attacker.getName() << " dealt " << cxxg::types::Color::RED
              << EAE.Damage << " damage" << cxxg::types::Color::NONE << " to "
              << EAE.Target.getName();
}

void EventHistoryWriter::onEntityDiedEvent(const EntityDiedEvent &EDE) {
  if (!EDE.isPlayer()) {
    return;
  }
  Hist.warn() << cxxg::types::Color::RED << "You died!";
}
