#include <rogue/JSONHelpers.h>
#include <ymir/Config/Types.hpp>

namespace rogue {

StatPoints parseStatPoints(const rapidjson::Value &V) {
  StatPoints P;
  P.Str = V["str"].GetInt();
  P.Dex = V["dex"].GetInt();
  P.Int = V["int"].GetInt();
  P.Vit = V["vit"].GetInt();
  return P;
}

Tile parseTile(const rapidjson::Value &V) {
  const auto Char = V["char"].GetString()[0];
  const auto Color = ymir::Config::parseRgbColor(V["color"].GetString());
  cxxg::types::RgbColor CxxColor{Color.R, Color.G, Color.B};

  if (V.HasMember("bg_color")) {
    const auto BgColor = ymir::Config::parseRgbColor(V["bg_color"].GetString());
    CxxColor.HasBackground = true;
    CxxColor.BgR = BgColor.R;
    CxxColor.BgG = BgColor.G;
    CxxColor.BgB = BgColor.B;
  }

  return Tile{{Char, CxxColor}};
}

} // namespace rogue