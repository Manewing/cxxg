#include <cxxg/Types.h>
#include <rogue/Level.h>
#include <rogue/Parser.h>
#include <ymir/Config/Parser.hpp>
#include <ymir/Config/Types.hpp>

namespace rogue {

cxxg::types::ColoredChar parseColoredChar(const std::string &Value) {
  static const std::regex Regex("\\{('..?'), (\"#[0-9a-fA-F]+\")\\}");
  std::smatch Match;
  if (std::regex_match(Value, Match, Regex)) {
    auto Char = ymir::Config::Parser::parseChar(Match[1]);
    // FIXME allow background colors
    auto Color = ymir::Config::parseRgbColor(Match[2]);
    cxxg::types::RgbColor CxxColor{Color.R, Color.G, Color.B};
    return cxxg::types::ColoredChar(Char, CxxColor);
  }
  throw std::runtime_error("Invalid colored char format: " + Value);
}

Tile parseTile(const std::string &Value) {
  auto ColoredChar = parseColoredChar(Value);
  return Tile{ColoredChar};
}

void registerParserTypes(ymir::Config::Parser &P) {
  P.registerType("CC", parseColoredChar);
  P.registerType("Tile", parseTile);
}

ymir::Config::AnyDict loadConfigurationFile(const std::filesystem::path &File) {
  ymir::Config::Parser CfgParser;
  ymir::Config::registerYmirTypes<int>(CfgParser);
  registerParserTypes(CfgParser);
  CfgParser.parse(File);
  return CfgParser.getCfg();
}

} // namespace rogue