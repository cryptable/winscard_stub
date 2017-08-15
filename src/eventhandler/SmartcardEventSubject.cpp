//
// Created by david on 8/11/17.
//

#include "SmartcardEventSubject.h"

#include <utility>

using namespace std;
using namespace readers;

namespace eventhandler {

  SmartcardEventSubject::SmartcardEventSubject(shared_ptr<SmartcardReader> rdr) : reader(std::move(rdr)) { }

  DWORD SmartcardEventSubject::getReaderState(LPSCARD_READERSTATE readerState) {
    if (string(readerState->szReader) == reader->getReaderIdentifier()) {
      readerState->szReader = reader->getReaderIdentifier().c_str();
      reader->getEventInfo(readerState);
      return SCARD_S_SUCCESS;
    }
    return static_cast<DWORD>(SCARD_E_UNKNOWN_READER);
  }
}