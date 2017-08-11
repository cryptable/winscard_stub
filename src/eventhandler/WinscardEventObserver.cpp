//
// Created by david on 11/08/17.
//

#include "WinscardEventObserver.h"

using namespace std;

namespace eventhandler {

  WinscardEventObserver::WinscardEventObserver(SCARD_READERSTATE *rgReaderStates, DWORD cReaders) :
      toScanReaders(rgReaderStates),
      numberOfToScanReaders(cReaders) {
  }

  void WinscardEventObserver::wait_infinite(mutex &m) {
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk);
  }

  void WinscardEventObserver::wait_for(mutex &m, unsigned int sec) {
    std::unique_lock<std::mutex> lk(m);
    cv.wait_for(lk, chrono::seconds(sec));
  }

  void WinscardEventObserver::update(WinscardEventSubject *event) {
    for (int i=0; i<numberOfToScanReaders; i++) {
      if (event->getReaderState(&(toScanReaders[i])) == SCARD_S_SUCCESS) {
        cv.notify_one();
      }
    }
  }

};
