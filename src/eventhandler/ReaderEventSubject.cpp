//
// Created by david on 8/11/17.
//

#include "ReaderEventSubject.h"

#include <utility>

using namespace std;
using namespace readers;

namespace eventhandler {

  ReaderEventSubject::ReaderEventSubject(shared_ptr<SmartcardReader> rdr) : reader(std::move(rdr)) {

  }

  DWORD ReaderEventSubject::getReaderState(LPSCARD_READERSTATE readerState)
  {
    if (string(readerState->szReader) == R"(\\?PnP?\Notification)") {
      readerState->szReader = reader->getReaderIdentifier().c_str();
      readerState->dwEventState = SCARD_STATE_CHANGED;
      return SCARD_S_SUCCESS;
    }
    return static_cast<DWORD>(SCARD_E_UNKNOWN_READER);
  }

}
