//
// Created by david on 8/11/17.
//

#ifndef WINSCARD_STUB_TESTSMARTCARD_H
#define WINSCARD_STUB_TESTSMARTCARD_H


#include <pcsclite.h>
#include "Smartcard.h"

namespace smartcards {
  /**
  * Implementation of a test smartcard
  */
  class TestSmartcard : public Smartcard {
  public:
    TestSmartcard();

    DWORD execute(SCARDHANDLE handle, const char *in_apdu, size_t in_apdu_lg, char **out_apdu, size_t *out_apd_lg) override;

  };

}

#endif //WINSCARD_STUB_TESTSMARTCARD_H
