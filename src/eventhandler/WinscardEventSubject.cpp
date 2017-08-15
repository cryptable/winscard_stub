//
// Created by david on 11/08/17.
//

#include "WinscardEventSubject.h"

using namespace std;

namespace eventhandler {

  WinscardEventSubject::WinscardEventSubject() = default;

  WinscardEventSubject::~WinscardEventSubject() = default;

  void WinscardEventSubject::attach(shared_ptr<WinscardEventObserver> observer) {
    std::lock_guard<std::mutex> lck (m);
    observers.insert(observer);
  }

  void WinscardEventSubject::deattach(shared_ptr<WinscardEventObserver> &observer) {
    std::lock_guard<std::mutex> lck (m);
    observers.erase(observer);
  }

  void WinscardEventSubject::notify() {
    std::lock_guard<std::mutex> lck (m);
    for (const auto &observer: observers) {
      observer->update(this);
    }
  }

};
