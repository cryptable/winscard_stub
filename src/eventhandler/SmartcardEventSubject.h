//
// Created by david on 8/11/17.
//

#ifndef WINSCARD_STUB_SMARTCARDEVENTSUBJECT_H
#define WINSCARD_STUB_SMARTCARDEVENTSUBJECT_H

#include "WinscardEventSubject.h"

namespace readers {
  class SmartcardReader;
};

namespace eventhandler {

  class SmartcardEventSubject : public WinscardEventSubject {

  public:
    void setReader(readers::SmartcardReader *rdr);

    DWORD getReaderState(LPSCARD_READERSTATE readerState) override;

  private:
    readers::SmartcardReader *reader;
  };

};
#endif //WINSCARD_STUB_SMARTCARDEVENTSUBJECT_H
