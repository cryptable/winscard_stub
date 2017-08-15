//
// Created by david on 8/11/17.
//

#include "NonPinpadReader.h"

namespace readers {

  NonPinpadReader::NonPinpadReader() : SmartcardReader("Non Pinpad Reader") {

  };

  DWORD NonPinpadReader::execute(const char *in_apdu, size_t in_apdu_lg, char **out_apdu, size_t *out_apd_lg) {
    return static_cast<DWORD>(SCARD_E_UNSUPPORTED_FEATURE);
  };

};
