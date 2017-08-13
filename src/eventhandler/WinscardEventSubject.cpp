//
// Created by david on 11/08/17.
//

#include "WinscardEventSubject.h"

using namespace std;

namespace eventhandler {

  void WinscardEventSubject::attach(shared_ptr<WinscardEventObserver> observer) {
    observers.push_back(observer);
  }

  void WinscardEventSubject::deattach(shared_ptr<WinscardEventObserver> observer) {

  }

  void WinscardEventSubject::notify() {
    for (auto observer: observers) {
      observer->update(this);
    }
  }

};
