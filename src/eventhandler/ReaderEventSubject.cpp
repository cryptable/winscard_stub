//
// Created by david on 8/11/17.
//

#include "ReaderEventSubject.h"

using namespace std;
using namespace readers;

namespace eventhandler {

  ReaderEventSubject::ReaderEventSubject(shared_ptr<SmartcardReader> rdr) : reader(rdr) {

  }

  DWORD ReaderEventSubject::getReaderState(LPSCARD_READERSTATE readerState)
  {
    if (string(readerState->szReader) == "\\\\?PnP?\\Notification") {
      readerState->szReader = reader->getReaderIdentifier().c_str();
      readerState->dwEventState = SCARD_STATE_CHANGED;
      return SCARD_S_SUCCESS;
    }
    return SCARD_E_UNKNOWN_READER;
  }

}
