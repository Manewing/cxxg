#include <rogue/UI/WordWrap.h>

namespace rogue::ui {

WordWrap::WordWrap(const std::string &Text, std::size_t LineWidth)
    : LineWidth(LineWidth), StrBuffer(Text) {

  // reserve least amount of lines needed
  LineIntvs.reserve(Text.size() / LineWidth + 1);

  std::size_t CurrentReadPos = 0;
  std::size_t CurrentWordSize = 0;
  std::size_t CurrentLineSize = 0;
  std::size_t LastLineEnd = 0;
  while (CurrentReadPos < Text.size()) {
    // not a whitespace character?
    if (Text[CurrentReadPos] != ' ' && Text[CurrentReadPos] != '\n') {
      // yes, increase current word size
      CurrentWordSize += 1;
    } else {
      // current line size with last word 'CurrentWordSize' and
      // whitespace greater than line size?
      if (CurrentLineSize + CurrentWordSize + 1 > LineWidth) {
        // add interval
        LineIntvs.push_back(Interval{LastLineEnd, CurrentLineSize});

        // set end of last line and reset current line size
        LastLineEnd += CurrentLineSize;
        CurrentLineSize = 0;
      }

      // add started word to current line
      CurrentLineSize += ++CurrentWordSize;
      CurrentWordSize = 0;

      if (Text[CurrentReadPos] == '\n') {
        // got new line -> forced cut
        LineIntvs.push_back(Interval{LastLineEnd, CurrentLineSize});
        StrBuffer[LastLineEnd + CurrentLineSize - 1] = ' ';
        LastLineEnd += CurrentLineSize;
        CurrentLineSize = 0;
      }
    }
    // increase reading pos
    CurrentReadPos++;
  }

  if (CurrentLineSize + CurrentWordSize + 1 > LineWidth) {
    LineIntvs.push_back(Interval{LastLineEnd, CurrentLineSize});
    LastLineEnd += CurrentLineSize;
    CurrentLineSize = 0;
  }

  CurrentLineSize += CurrentWordSize;
  LineIntvs.push_back(Interval{LastLineEnd, CurrentLineSize + 1});
  StrBuffer.push_back(' ');
}

} // namespace rogue::ui