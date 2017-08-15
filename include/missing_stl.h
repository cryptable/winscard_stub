//
// Created by david on 8/5/17.
//

#ifndef WINSCARD_STUB_UTILITIES_H
#define WINSCARD_STUB_UTILITIES_H

#include <utility>
#include <memory>

template<typename T, typename... Args>std::unique_ptr<T> make_unique(Args&&... args) {
  return std::unique_ptr<T>{new T{std::forward<Args>(args)...}};
};

#endif //WINSCARD_STUB_UTILITIES_H
