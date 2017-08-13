//
// Created by david on 11/08/17.
//

#ifndef WINSCARD_STUB_WINSCARDEVENTOBSERVER_H
#define WINSCARD_STUB_WINSCARDEVENTOBSERVER_H

#include <pcsclite.h>
#include <mutex>
#include <condition_variable>

namespace eventhandler {

  class WinscardEventSubject;

  class WinscardEventObserver {
  public:

    explicit WinscardEventObserver(SCARD_READERSTATE *rgReaderStates, DWORD cReaders);

    void wait_infinite(std::mutex &m);

    void wait_for(std::mutex &m, unsigned int sec);

    void update(WinscardEventSubject *event);

  private:
    std::condition_variable cv;
    SCARD_READERSTATE  *toScanReaders;
    DWORD              numberOfToScanReaders;
  };

};

#endif //WINSCARD_STUB_WINSCARDEVENTOBSERVER_H
