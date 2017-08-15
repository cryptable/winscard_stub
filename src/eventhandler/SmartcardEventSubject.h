//
// Created by david on 8/11/17.
//

#ifndef WINSCARD_STUB_SMARTCARDEVENTSUBJECT_H
#define WINSCARD_STUB_SMARTCARDEVENTSUBJECT_H

#include "WinscardEventSubject.h"
#include "../readers/SmartcardReader.h"

namespace eventhandler {

  class SmartcardEventSubject : public WinscardEventSubject {

  public:
    explicit SmartcardEventSubject(std::shared_ptr<readers::SmartcardReader> rdr);

    DWORD getReaderState(LPSCARD_READERSTATE readerState) override;

  private:
    std::shared_ptr<readers::SmartcardReader> reader;
  };

};
#endif //WINSCARD_STUB_SMARTCARDEVENTSUBJECT_H
