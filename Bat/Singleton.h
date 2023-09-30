#pragma once

namespace bat {

template <typename T, auto... ARGS> struct Singleton {
  static T &_instance() {
    static T _instance{ARGS...};
    return _instance;
  }
  T &operator*() const { return _instance(); }
  T *operator->() const { return &_instance(); }
};

} // namespace bat
