//
// Created by david on 11/08/17.
//

#ifndef WINSCARD_STUB_WINSCARDEVENTSUBJECT_H
#define WINSCARD_STUB_WINSCARDEVENTSUBJECT_H

#include <set>
#include "WinscardEventObserver.h"

namespace eventhandler {

  class WinscardEventSubject {

  public:
    WinscardEventSubject(const WinscardEventSubject&) = delete;
    WinscardEventSubject(WinscardEventSubject&&) = delete;
    WinscardEventSubject &operator=(const WinscardEventSubject&) = delete;
    WinscardEventSubject &operator=(WinscardEventSubject&&) = delete;

    virtual ~WinscardEventSubject() = 0;

    void attach(std::shared_ptr<WinscardEventObserver> observer);

    void deattach(std::shared_ptr<WinscardEventObserver> &observer);

    void notify();

    virtual DWORD getReaderState(LPSCARD_READERSTATE readerState) = 0;

  protected:
    WinscardEventSubject();

  private:
    std::mutex m;
    std::set<std::shared_ptr<WinscardEventObserver>> observers;
  };

};

#endif //WINSCARD_STUB_WINSCARDEVENTSUBJECT_H
