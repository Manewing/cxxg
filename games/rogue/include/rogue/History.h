#ifndef ROGUE_HISTORY_H
#define ROGUE_HISTORY_H

#include <cxxg/Row.h>
#include <rogue/EventHub.h>
#include <sstream>
#include <string>
#include <vector>

namespace rogue {
class Game;
class History;
struct DebugMessageEvent;
struct EntityAttackEvent;
struct EntityDiedEvent;
struct BuffExpiredEvent;
struct PlayerInfoMessageEvent;
struct WarningMessageEvent;
struct ErrorMessageEvent;
} // namespace rogue

namespace rogue {

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
  struct Message {
    // FIXME refactor to decouple from cxxg
    cxxg::Row Row;

    /// Number of times this message has been repeated
    unsigned Count = 1;
  };

public:
  explicit History(Game &G);
  HistoryMessageAssembler info();
  HistoryMessageAssembler warn();

  const std::vector<Message> &getMessages() const { return Messages; }

private:
  void addMessage(const cxxg::Row &Msg);

private:
  Game &G;
  std::vector<Message> Messages;
};

class EventHistoryWriter : public EventHubConnector {
public:
  explicit EventHistoryWriter(History &Hist, bool Debug = false);
  void setEventHub(EventHub *Hub) final;

private:
  void onEntityAttackEvent(const EntityAttackEvent &EAE);
  void onEntityDiedEvent(const EntityDiedEvent &EDE);
  void onBuffExpiredEvent(const BuffExpiredEvent &BEE);
  void onPlayerInfoMessageEvent(const PlayerInfoMessageEvent &PIME);
  void onWarningMessageEvent(const WarningMessageEvent &WarnEv);
  void onErrorMessageEvent(const ErrorMessageEvent &ErrEv);
  void onDebugMessageEvent(const DebugMessageEvent &DbgEv);

private:
  History &Hist;
  bool Debug = false;
};

} // namespace rogue

#endif // #ifndef ROGUE_HISTORY_H