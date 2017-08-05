//
// Created by david on 8/5/17.
//

#ifndef WINSCARD_STUB_UTILITIES_H
#define WINSCARD_STUB_UTILITIES_H

#include <utility>
#include <memory>

namespace std {
  template<typename T, typename... Args>unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>{new T{std::forward<Args>(args)...}};
  };
};

#endif //WINSCARD_STUB_UTILITIES_H
