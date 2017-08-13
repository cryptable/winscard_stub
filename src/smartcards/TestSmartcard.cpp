//
// Created by david on 8/11/17.
//

#include "TestSmartcard.h"

using namespace std;

namespace smartcards {
  TestSmartcard::TestSmartcard() : Smartcard( vector<unsigned char>({ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16 }) ) {

  };

  DWORD TestSmartcard::execute(SCARDHANDLE handle, const char *in_apdu, size_t in_apdu_lg, char **out_apdu, size_t *out_apd_lg) {
      return static_cast<DWORD>(SCARD_E_UNSUPPORTED_FEATURE);
  };

}
