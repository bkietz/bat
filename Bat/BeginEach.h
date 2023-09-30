#pragma once

#include <type_traits>

namespace bat {

template <typename... A> void begin_each(A &&...args) {
  auto begin_one = [](auto &&arg) {
    if constexpr (std::is_invocable_v<decltype(arg)>) {
      arg();
    } else {
      arg.begin();
    }
  };
  (begin_one(args), ...);
}

} // namespace bat
