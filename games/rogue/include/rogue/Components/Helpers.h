#ifndef ROGUE_COMPONENTS_HELPERS_H
#define ROGUE_COMPONENTS_HELPERS_H

#include <entt/entt.hpp>

namespace rogue {

template <typename... Args> struct ComponentList {
  using List = std::tuple<Args...>;

  static bool validate(entt::registry &Reg, entt::entity Et) {
    return Reg.all_of<Args...>(Et);
  }
};

template <typename ComponentListT, typename Callable>
auto applyForComponents(Callable C) {
  return std::apply([&C](auto &&...Args) { (C(Args), ...); },
                    typename ComponentListT::List{});
}

template <typename ComponentListT>
void addComponents(entt::entity Entity, entt::registry &Reg) {
  applyForComponents<ComponentListT>([&Entity, &Reg](auto &Arg) {
    Reg.emplace<std::decay_t<decltype(Arg)>>(Entity);
  });
}

template <typename T>
void copyComponent(entt::entity EntityFrom, entt::registry &RegFrom,
                   entt::entity EntityTo, entt::registry &RegTo) {
  if (auto *SBC = RegFrom.try_get<T>(EntityFrom)) {
    RegTo.emplace<T>(EntityTo, *SBC);
  }
}

template <typename ComponentListT>
void copyComponents(entt::entity EntityFrom, entt::registry &RegFrom,
                    entt::entity EntityTo, entt::registry &RegTo) {
  applyForComponents<ComponentListT>(
      [&EntityFrom, &RegFrom, &EntityTo, &RegTo](auto &Arg) {
        copyComponent<std::decay_t<decltype(Arg)>>(EntityFrom, RegFrom,
                                                   EntityTo, RegTo);
      });
}

template <typename T>
void copyComponentOrFail(entt::entity EntityFrom, entt::registry &RegFrom,
                         entt::entity EntityTo, entt::registry &RegTo) {
  assert(RegFrom.any_of<T>(EntityFrom));
  if constexpr (sizeof(T) == 1) {
    RegTo.emplace<T>(EntityTo);
  } else {
    RegTo.emplace<T>(EntityTo, RegFrom.get<T>(EntityFrom));
  }
}

template <typename ComponentListT>
void copyComponentsOrFail(entt::entity EntityFrom, entt::registry &RegFrom,
                          entt::entity EntityTo, entt::registry &RegTo) {
  applyForComponents<ComponentListT>(
      [&EntityFrom, &RegFrom, &EntityTo, &RegTo](auto &Arg) {
        copyComponentOrFail<std::decay_t<decltype(Arg)>>(EntityFrom, RegFrom,
                                                         EntityTo, RegTo);
      });
}

template <typename T>
void removeComponent(entt::entity Entity, entt::registry &Reg) {
  if (Reg.any_of<T>(Entity)) {
    Reg.remove<T>(Entity);
  }
}

template <typename ComponentListT>
void removeComponents(entt::entity Entity, entt::registry &Reg) {
  applyForComponents<ComponentListT>([&Entity, &Reg](auto &Arg) {
    removeComponent<std::decay_t<decltype(Arg)>>(Entity, Reg);
  });
}

} // namespace rogue

#endif // #ifndef ROGUE_COMPONENTS_HELPERS_H