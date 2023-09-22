#ifndef ROGUE_COMPONENTS_HELPERS_H
#define ROGUE_COMPONENTS_HELPERS_H

#include <entt/entt.hpp>

namespace rogue {

template <typename... Args> struct ComponentList {
  using List = std::tuple<Args...>;
};

template <typename T>
void copyComponent(entt::entity EntityFrom, entt::registry &RegFrom,
                   entt::entity EntityTo, entt::registry &RegTo) {
  if (auto *SBC = RegFrom.try_get<T>(EntityFrom)) {
    RegTo.emplace<T>(EntityTo, *SBC);
  }
}

template <typename T>
void copyComponents(entt::entity EntityFrom, entt::registry &RegFrom,
                    entt::entity EntityTo, entt::registry &RegTo) {
  std::apply(
      [&EntityFrom, &RegFrom, &EntityTo, &RegTo](const auto &...Args) {
        (copyComponent<std::decay_t<decltype(Args)>>(EntityFrom, RegFrom,
                                                     EntityTo, RegTo),
         ...);
      },
      typename T::List{});
}

template <typename T>
void copyComponentOrFail(entt::entity EntityFrom, entt::registry &RegFrom,
                         entt::entity EntityTo, entt::registry &RegTo) {
  assert(RegFrom.any_of<T>(EntityFrom));
  RegTo.emplace<T>(EntityTo, RegFrom.get<T>(EntityFrom));
}

template <typename T>
void copyComponentsOrFail(entt::entity EntityFrom, entt::registry &RegFrom,
                          entt::entity EntityTo, entt::registry &RegTo) {
  std::apply(
      [&EntityFrom, &RegFrom, &EntityTo, &RegTo](const auto &...Args) {
        (copyComponentOrFail<std::decay_t<decltype(Args)>>(EntityFrom, RegFrom,
                                                           EntityTo, RegTo),
         ...);
      },
      typename T::List{});
}

template <typename T>
void removeComponent(entt::entity Entity, entt::registry &Reg) {
  if (Reg.any_of<T>(Entity)) {
    Reg.remove<T>(Entity);
  }
}

template <typename T>
void removeComponents(entt::entity Entity, entt::registry &Reg) {
  std::apply(
      [&Entity, &Reg](const auto &...Args) {
        (removeComponent<std::decay_t<decltype(Args)>>(Entity, Reg), ...);
      },
      typename T::List{});
}

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_HELPERS_H