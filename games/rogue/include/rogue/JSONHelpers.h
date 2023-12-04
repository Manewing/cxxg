#ifndef ROGUE_JSON_HELPERS_H
#define ROGUE_JSON_HELPERS_H

#include <rogue/Components/Stats.h>
#include <rogue/JSON.h>
#include <rogue/Tile.h>

namespace rogue {

StatPoints parseStatPoints(const rapidjson::Value &V);

Tile parseTile(const rapidjson::Value &V);

template <typename T>
void parseReductionBuff(const rapidjson::Value &V, T &Buff) {
  auto TickAmount = V["reduce_amount"].GetDouble();
  auto TickPeriod = V["tick_period"].GetUint();
  auto RealDuration = TickPeriod * V["ticks"].GetUint();
  Buff.init(TickAmount, RealDuration, TickPeriod);
}

template <typename T>
void parseRegenerationBuff(const rapidjson::Value &V, T &Buff) {
  auto TickAmount = V["regen_amount"].GetDouble();
  auto TickPeriod = V["tick_period"].GetUint();
  auto RealDuration = TickPeriod * V["ticks"].GetUint();
  Buff.init(TickAmount, RealDuration, TickPeriod);
}

} // namespace rogue

#endif // #ifndef ROGUE_JSON_HELPERS_H