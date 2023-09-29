#include <cxxg/Screen.h>
#include <cxxg/Utils.h>
#include <rogue/History.h>
#include <rogue/UI/Frame.h>
#include <rogue/UI/History.h>

namespace rogue::ui {

HistoryController::HistoryController(cxxg::types::Position Pos,
                                     const History &Hist,
                                     unsigned NumHistoryRows)
    : Widget(Pos), Hist(Hist), NumHistoryRows(NumHistoryRows) {
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
  case cxxg::utils::KEY_ESC:
  case 'h':
    return false;
  default:
    break;
  }
  return true;
}

std::string HistoryController::getInteractMsg() const {
  return "[^/v] Navigate";
}

void HistoryController::draw(cxxg::Screen &Scr) const {
  const int NumHistoryRows = 18;
  std::string_view Header = "History";

  // Draw header
  Frame::drawFrameHeader(Scr, {Pos.X, Pos.Y}, Header, Scr.getSize().X,
                         cxxg::types::Color::NONE, cxxg::types::Color::NONE);

  const auto &Msgs = Hist.getMessages();
  for (unsigned int Idx = 0; Idx < NumHistoryRows; Idx++) {
    const int LinePos = Pos.Y + Idx + 1;
    const unsigned MsgPos = Idx + Offset;

    if (MsgPos >= Msgs.size()) {
      Scr[LinePos][Pos.X] << std::string(Scr.getSize().X, ' ');
      continue;
    }

    if (Idx == 0 && Offset != 0) {
      Scr[LinePos][Pos.X] << std::string(Scr.getSize().X, ' ');
      Scr[LinePos][Pos.X + Scr.getSize().X / 2] = '^';
      continue;
    }

    if (Idx == NumHistoryRows - 1 && MsgPos < Msgs.size() - 1) {
      Scr[LinePos][Pos.X] << std::string(Scr.getSize().X, ' ');
      Scr[LinePos][Pos.X + Scr.getSize().X / 2] = 'v';
      continue;
    }

    Scr[LinePos][Pos.X] << Msgs.at(MsgPos);
  }

  // Draw footer
  const unsigned Start =
      std::min(Msgs.size(), static_cast<std::size_t>(Offset + 1));
  const unsigned End =
      std::min(Msgs.size(), static_cast<std::size_t>(Offset + NumHistoryRows));
  std::string Footer = std::to_string(Start) + "-" + std::to_string(End);

  Frame::drawFrameHeader(Scr, {Pos.X, Pos.Y + NumHistoryRows + 1}, Footer,
                         Scr.getSize().X, cxxg::types::Color::NONE,
                         cxxg::types::Color::NONE);
}

} // namespace rogue::ui