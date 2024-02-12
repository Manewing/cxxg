#ifndef ROGUE_STRONG_TYPE_H
#define ROGUE_STRONG_TYPE_H

#include <type_traits>

/// Defines a strong type with the given name. This is used for defining, e.g.
/// scalar Ids that should only be explicitly converted to.
#define ROGUE_STRONG_TYPE(Type, Name)                                          \
  struct Name {                                                                \
    using IsNoThrowConstructible = std::is_nothrow_constructible<Type>;        \
    constexpr Name() noexcept(IsNoThrowConstructible::value) = default;        \
    constexpr explicit Name(Type Value) noexcept(                              \
        IsNoThrowConstructible::value)                                         \
        : Value(Value) {}                                                      \
    constexpr operator const Type &() const { return Value; }                  \
    constexpr operator Type &() { return Value; }                              \
    Type Value = Type();                                                       \
    template <class Archive> void serialize(Archive &Ar) { Ar(Value); }        \
  }

#endif // #ifndef ROGUE_STRONG_TYPE_H