#ifndef ROGUE_UI_WORD_WRAP_H
#define ROGUE_UI_WORD_WRAP_H

#include <string>
#include <vector>

namespace rogue::ui {

class WordWrap {
public:
  struct Interval {
    std::size_t Start = 0;
    std::size_t End = 0;
  };
  using LineIntervals = std::vector<Interval>;

public:
  WordWrap(const std::string &Text, std::size_t LineWidth);

  inline auto getLineWidth() const { return LineWidth; }
  inline auto getNumLines() const { return LineIntvs.size(); }

  inline auto getLine(std::size_t Line) const {
    auto It = LineIntvs.at(Line);
    return std::string_view(StrBuffer).substr(It.Start, It.End);
  }

private:
  std::size_t LineWidth = 0;
  LineIntervals LineIntvs;
  std::string StrBuffer;
};

} // namespace rogue::ui

#endif // #ifndef ROGUE_UI_WORD_WRAP_H