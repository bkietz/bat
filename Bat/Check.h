#pragma once

#include <type_traits>

namespace bat {

/// Check can be used to determine whether expressions would be valid.
/// (Wraps SFINAE in as little boilerplate as possible).
/// Declare a Check like:
///
///     Check constexpr has_a = [](auto x) -> decltype(x.a) {};
///
/// After which it can be used like:
///
///     struct { int a; } does;
///     static_assert(has_a(does));
///
///     struct { int b; } does_not;
///     static_assert(not has_a(does_not));
///
/// To write a check which requires that an expression be truthy in addition to
/// being valid:
///
///     Check constexpr value_3 = [](auto x) -> decltype(requires_<value(x) ==
///     3>) {};
///
/// To write a check which requires multiple expressions be valid, separate them
/// with commas inside the `decltype()`:
///
///     Check constexpr has_a_and_b = [](auto x) -> decltype(x.a, x.b) {};
///
/// To write a check which accepts multiple branches, construct the Check with
/// an initializer list:
///
///     Check constexpr has_a_or_b{
///       [](auto x) -> decltype(x.a) {},
///       [](auto x) -> decltype(x.b) {},
///     };
///
/// If you don't want to declare the Check as a constexpr variable for some
/// reason, you can use it as a temporary instead:
///
///    int i;
///    static_assert(Check([](auto x) -> decltype(x + 1) {})(i));
///
/// Probably *don't* use this for conditions in enable_if. If you find yourself
/// trying to do this, probably you want to just use decltype() directly:
///
///    - template <typename T>
///    - enable_if_t<has_a(declval<T>()), // require that T has data member a
///    -             T> frobnicate_a(T);
///    + template <typename T, typename DisableUnlessHasA = decltype(&T::a)>
///    + T frobnicate_a(T t);
///
/// Loosely inspired by Boost.Hana's is_valid().
template <typename... Fn> struct Check {
  constexpr Check(Fn...) {} // NOLINT(google-explicit-constructor)

  template <typename... Args>
  constexpr std::bool_constant<(... or std::is_invocable_v<Fn, Args &&...>)>
  operator()(Args &&...) const {
    return {};
  }
};

template <bool Condition, typename = std::enable_if_t<Condition>>
static constexpr bool requires_ = true;

} // namespace bat
