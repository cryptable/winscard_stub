//
// Created by david on 8/11/17.
//

#ifndef WINSCARD_STUB_NONPINPADREADER_H
#define WINSCARD_STUB_NONPINPADREADER_H

#include "SmartcardReader.h"

namespace readers {

  /**
   * Non Pinpad Smartcard reader implementation
   */
  class NonPinpadReader : public SmartcardReader {

  public:
    /**
     * Constructor to instantiate the Non Pinpad Reader object
     */
    NonPinpadReader();

    /**
     * Specific implementation of the execute command for the Non Pinpad Reader. Commands to the smartcard will be proxied
     * by this function to the smartcard.
     *
     * @param scardhandle handle to the smartcard
     * @param in_apdu APDU command
     * @param in_apdu_lg length of the APDU command
     * @param out_apdu APDU result
     * @param out_apd_lg length of the APDU result
     * @return SCARD_S_SUCCESS
     */
    DWORD execute(const char *in_apdu, size_t in_apdu_lg, char **out_apdu, size_t *out_apd_lg) override;

  private:
  };

};


#endif //WINSCARD_STUB_NONPINPADREADER_H
