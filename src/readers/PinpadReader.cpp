//
// Created by david on 8/11/17.
//

#include "PinpadReader.h"

namespace readers {

  PinpadReader::PinpadReader(): SmartcardReader("Pinpad Reader") {

  }

  DWORD PinpadReader::execute(SCARDHANDLE scardhandle, const char *in_apdu, size_t in_apdu_lg, char **out_apdu, size_t *out_apd_lg) {
    return static_cast<DWORD>(SCARD_E_UNSUPPORTED_FEATURE);
  }

};
