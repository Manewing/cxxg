#ifndef ROGUE_HISTORY_H
#define ROGUE_HISTORY_H

#include <cxxg/Row.h>
#include <sstream>
#include <string>
#include <vector>

#include "Entity.h"

class Game;
class History;
struct EntityDiedEvent;

class HistoryMessageAssembler {
public:
  HistoryMessageAssembler(History &Hist, cxxg::RowAccessor Row);
  HistoryMessageAssembler(HistoryMessageAssembler &&);
  ~HistoryMessageAssembler();

  template <typename Type> HistoryMessageAssembler &operator<<(const Type &T) {
    Row << T;
    return *this;
  }

private:
  History &Hist;
  cxxg::RowAccessor Row;
};

class History {
  friend class HistoryMessageAssembler;

public:
  explicit History(Game &G);
  HistoryMessageAssembler info();
  HistoryMessageAssembler warn();

  const std::vector<cxxg::Row> &getMessages() const { return Messages; }

private:
  void addMessage(const cxxg::Row &Msg);

private:
  Game &G;
  std::vector<cxxg::Row> Messages;
};

struct DebugMessageEvent : public Event {
  std::stringstream Message;

  template <typename Type> DebugMessageEvent &operator<<(const Type &T) {
    Message << T;
    return *this;
  }
};

class EventHistoryWriter : public EventHubConnector {
public:
  explicit EventHistoryWriter(History &Hist);
  void setEventHub(EventHub *Hub) final;

private:
  void onEntityAttackEvent(const EntityAttackEvent &EAE);
  void onEntityDiedEvent(const EntityDiedEvent &EDE);
  void onDebugMessageEvent(const DebugMessageEvent &DbgEv);

private:
  History &Hist;
};

#endif // #ifndef ROGUE_HISTORY_H