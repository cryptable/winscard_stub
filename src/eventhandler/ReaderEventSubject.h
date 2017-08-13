//
// Created by david on 8/11/17.
//

#ifndef WINSCARD_STUB_READEREVENTSUBJECT_H
#define WINSCARD_STUB_READEREVENTSUBJECT_H

#include "WinscardEventSubject.h"
#include "../readers/SmartcardReader.h"

namespace eventhandler {

class ReaderEventSubject : public WinscardEventSubject {

public:
  explicit ReaderEventSubject(std::shared_ptr<readers::SmartcardReader> rdr);

  virtual DWORD getReaderState(LPSCARD_READERSTATE readerState);

private:
  std::shared_ptr<readers::SmartcardReader> reader;
};

};

#endif //WINSCARD_STUB_READEREVENTSUBJECT_H
