#pragma once

#include <tuple>
#include <cstddef>

struct range {
  range(int end) : end_{end} {}
  range(int begin, int end) : begin_{begin}, end_{end} {}

  struct iterator {
    int operator*() const { return i; }
    iterator &operator++() {
      ++i;
      return *this;
    }
    bool operator!=(iterator const &other) const { return i != other.i; }
    int i;
  };

  iterator begin() const { return {begin_}; }
  iterator end() const { return {end_}; }

  int begin_ = 0, end_;
};

template <typename Ranges, typename Indices> struct zip;

template <typename... Ranges>
zip(Ranges &&...)
    -> zip<std::tuple<Ranges...>, std::index_sequence_for<Ranges...>>;

template <typename... Ranges, size_t... I>
struct zip<std::tuple<Ranges...>, std::index_sequence<I...>> {
  explicit zip(Ranges... ranges) : ranges_(std::forward<Ranges>(ranges)...) {}

  std::tuple<Ranges...> ranges_;

  using sentinel = std::tuple<decltype(std::get<I>(ranges_).end())...>;

  struct iterator : std::tuple<decltype(std::get<I>(ranges_).begin())...> {
    using std::tuple<decltype(std::get<I>(ranges_).begin())...>::tuple;

    auto operator*() -> std::tuple<decltype(*std::get<I>(*this))...> {
      return {*std::get<I>(*this)...};
    }

    iterator &operator++() {
      (++std::get<I>(*this), ...);
      return *this;
    }

    bool operator!=(sentinel const &s) const {
      bool all_iterators_valid =
          (... && (std::get<I>(*this) != std::get<I>(s)));
      return all_iterators_valid;
    }
  };

  iterator begin() { return {std::get<I>(ranges_).begin()...}; }

  sentinel end() { return {std::get<I>(ranges_).end()...}; }
};
