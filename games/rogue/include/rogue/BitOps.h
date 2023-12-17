#ifndef ROGUE_BIT_OPS_H
#define ROGUE_BIT_OPS_H

#include <type_traits>

#define ROGUE_BIT_OPS_TYPE(T)                                                  \
  template <> struct _rogue_bit_ops_type<T> {                                  \
    static const bool enabled = true;                                          \
  };

template <typename T> struct _rogue_bit_ops_type {
  static const bool enabled = false;
};

template <typename T>
constexpr typename std::enable_if<_rogue_bit_ops_type<T>::enabled, T>::type
operator|(T Lhs, T Rhs) {
  using U = typename T::value_type;
  return T(static_cast<U>(Lhs) | static_cast<U>(Rhs));
}

template <typename T>
constexpr typename std::enable_if<_rogue_bit_ops_type<T>::enabled, T>::type
operator&(T Lhs, T Rhs) {
  using U = typename T::value_type;
  return T(static_cast<U>(Lhs) & static_cast<U>(Rhs));
}

template <typename T>
constexpr typename std::enable_if<_rogue_bit_ops_type<T>::enabled, T>::type
operator^(T Lhs, T Rhs) {
  using U = typename T::value_type;
  return T(static_cast<U>(Lhs) ^ static_cast<U>(Rhs));
}

template <typename T>
constexpr typename std::enable_if<_rogue_bit_ops_type<T>::enabled, T>::type
operator~(T Rhs) {
  using U = typename T::value_type;
  return T(~static_cast<U>(Rhs));
}

template <typename T>
constexpr typename std::enable_if<_rogue_bit_ops_type<T>::enabled, T>::type &
operator|=(T &Lhs, const T &Rhs) {
  using U = typename T::value_type;
  Lhs = T(static_cast<U>(Lhs) | static_cast<U>(Rhs));
  return Lhs;
}

template <typename T>
constexpr typename std::enable_if<_rogue_bit_ops_type<T>::enabled, T>::type &
operator&=(T &Lhs, const T &Rhs) {
  using U = typename T::value_type;
  Lhs = T(static_cast<U>(Lhs) & static_cast<U>(Rhs));
  return Lhs;
}

template <typename T>
constexpr typename std::enable_if<_rogue_bit_ops_type<T>::enabled, T>::type &
operator^=(T &Lhs, const T &Rhs) {
  using U = typename T::value_type;
  Lhs = T(static_cast<U>(Lhs) ^ static_cast<U>(Rhs));
  return Lhs;
}

#endif // #ifndef ROGUE_BIT_OPS_H