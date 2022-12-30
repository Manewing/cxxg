#ifndef ROGUE_HISTORY_H
#define ROGUE_HISTORY_H

#include <cxxg/Row.h>
#include <sstream>
#include <string>
#include <vector>

class Game;
class History;

class HistoryMessageAssembler {
public:
  HistoryMessageAssembler(History &Hist, cxxg::RowAccessor Row);
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

#endif // #ifndef ROGUE_HISTORY_H