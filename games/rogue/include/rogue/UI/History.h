#ifndef ROGUE_UI_HISTORY_H
#define ROGUE_UI_HISTORY_H

#include <rogue/UI/Widget.h>
#include <string_view>

namespace rogue {
class History;
} // namespace rogue

namespace rogue::ui {

class HistoryController : public Widget {
public:
  HistoryController(cxxg::types::Position Pos, const History &Hist,
                    unsigned NumHistoryRows = 18);
  bool handleInput(int Char) final;
  std::string getInteractMsg() const final;
  void draw(cxxg::Screen &Scr) const final;

protected:
  const History &Hist;
  const unsigned NumHistoryRows;
  unsigned Offset = 0;
};

} // namespace rogue::ui

#endif // #define ROGUE_UI_HISTORY_H