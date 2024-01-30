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

  auto checkAndAddLine = [&]() {
    // current line size with last word 'CurrentWordSize' and
    // whitespace greater than line size?
    if (CurrentLineSize + CurrentWordSize > LineWidth) {
      // add interval
      LineIntvs.push_back(Interval{LastLineEnd, CurrentLineSize});

      // set end of last line and reset current line size
      LastLineEnd += CurrentLineSize;
      CurrentLineSize = 0;
    }
  };

  auto checkAndCutWord = [&]() {
    // check if the word fits at all into a line
    if (CurrentWordSize > LineWidth) {
      // word is longer than a line -> cut it, since we anyways cut it add as
      // much as possible to the current line, then add as many lines as
      // needed
      // add as much as possible to current line
      auto LineLeft = static_cast<int>(LineWidth - CurrentLineSize);
      if (LineLeft > 0) {
        LineIntvs.push_back(Interval{LastLineEnd, LineWidth});
        LastLineEnd += LineWidth;
        CurrentLineSize = 0;
        CurrentWordSize -= LineLeft;
      }

      // add as many lines as needed
      while (CurrentWordSize > LineWidth) {
        LineIntvs.push_back(Interval{LastLineEnd, LineWidth});
        LastLineEnd += LineWidth;
        CurrentWordSize -= LineWidth;
      }
    }
  };

  while (CurrentReadPos < Text.size()) {
    // not a whitespace character?
    if (Text[CurrentReadPos] != ' ' && Text[CurrentReadPos] != '\n') {
      // yes, increase current word size
      CurrentWordSize += 1;
    } else {
      // handle cutting the word if it is to large
      checkAndCutWord();

      // handle adding a new line to wrap the word if needed
      checkAndAddLine();

      // add started word to current line
      CurrentLineSize += CurrentWordSize + 1;
      CurrentWordSize = 0;

      if (Text[CurrentReadPos] == '\n') {
        // got new line character -> forced new line
        LineIntvs.push_back(Interval{LastLineEnd, CurrentLineSize - 1});
        StrBuffer[LastLineEnd + CurrentLineSize - 1] = ' ';
        LastLineEnd += CurrentLineSize;
        CurrentLineSize = 0;
      }
    }
    // increase reading pos
    CurrentReadPos++;
  }

  // handle cutting the word if it is to large
  checkAndCutWord();

  // handle adding a new line to wrap the word if needed
  checkAndAddLine();

  // Create an interval for the last line, if the last line is empty, add a
  // whitespace so we do not have an empty string
  CurrentLineSize += CurrentWordSize;
  if (CurrentLineSize == 0) {
    StrBuffer.push_back(' ');
    CurrentLineSize++;
  }
  LineIntvs.push_back(Interval{LastLineEnd, CurrentLineSize});
}

} // namespace rogue::ui