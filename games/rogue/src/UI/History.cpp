#include <cxxg/Screen.h>
#include <cxxg/Utils.h>
#include <rogue/History.h>
#include <rogue/UI/Frame.h>
#include <rogue/UI/History.h>

namespace rogue::ui {

HistoryController::HistoryController(const History &Hist,
                                     unsigned NumHistoryRows)
    : Hist(Hist), NumHistoryRows(NumHistoryRows) {
  auto NumMsgs = Hist.getMessages().size();
  if (NumMsgs > NumHistoryRows) {
    Offset = Hist.getMessages().size() - NumHistoryRows;
  }
}

bool HistoryController::handleInput(int Char) {
  switch (Char) {
  case cxxg::utils::KEY_DOWN:
    if (!Hist.getMessages().empty() && Offset < Hist.getMessages().size() - 2) {
      Offset++;
    }
    break;
  case cxxg::utils::KEY_UP:
    if (Offset > 0) {
      Offset--;
    }
    break;
  case 'h':
    return false;
  default:
    break;
  }
  return true;
}

std::string_view HistoryController::getInteractMsg() const {
  return "[^/v] Navigate";
}

void HistoryController::draw(cxxg::Screen &Scr) const {
  const int PosX = 0, PosY = 2;
  const unsigned NumHistoryRows = 18;
  std::string_view Header = "History";

  // Draw header
  Frame::drawFrameHeader(Scr, {PosX, PosY}, Header, Scr.getSize().X);

  const auto &Msgs = Hist.getMessages();
  for (unsigned int Idx = 0; Idx < NumHistoryRows; Idx++) {
    const int LinePos = PosY + Idx + 1;
    const unsigned MsgPos = Idx + Offset;

    if (MsgPos >= Msgs.size()) {
      Scr[LinePos][PosX] << std::string(Scr.getSize().X, ' ');
      continue;
    }

    if (Idx == 0 && Offset != 0) {
      Scr[LinePos][PosX] << std::string(Scr.getSize().X, ' ');
      Scr[LinePos][PosX + Scr.getSize().X / 2] = '^';
      continue;
    }

    if (Idx == NumHistoryRows - 1 && MsgPos < Msgs.size() - 1) {
      Scr[LinePos][PosX] << std::string(Scr.getSize().X, ' ');
      Scr[LinePos][PosX + Scr.getSize().X / 2] = 'v';
      continue;
    }

    Scr[LinePos] = Msgs.at(MsgPos);
  }

  // Draw footer
  const unsigned Start =
      std::min(Msgs.size(), static_cast<std::size_t>(Offset + 1));
  const unsigned End =
      std::min(Msgs.size(), static_cast<std::size_t>(Offset + NumHistoryRows));
  std::string Footer = std::to_string(Start) + "-" + std::to_string(End);

  Frame::drawFrameHeader(Scr, {PosX, PosY + NumHistoryRows + 1}, Footer,
                         Scr.getSize().X);
}

} // namespace rogue::ui