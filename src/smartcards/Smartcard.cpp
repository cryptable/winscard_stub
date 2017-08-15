//
// Created by david on 8/11/17.
//

#include "Smartcard.h"
#include "TestSmartcard.h"
#include "missing_stl.h"

using namespace std;
using namespace smartcards;

namespace smartcards {

  Smartcard::Smartcard(const vector<unsigned  char> &atr, DWORD dwSharingMode, DWORD dwProtocol) :
    allowedSharingModes(dwSharingMode),
    allowedProtocol (dwProtocol),
    disposition(SCARD_LEAVE_CARD),
    transaction(false),
    ATR(atr) {
  }

  DWORD Smartcard::connect(DWORD dwShareMode, DWORD dwPreferredProtocols, LPDWORD pdwActiveProtocol) {

    if (dwShareMode != allowedSharingModes) {
      return static_cast<DWORD>(SCARD_E_INVALID_VALUE);
    }
    if ((dwPreferredProtocols & allowedProtocol) != allowedProtocol) {
      return static_cast<DWORD>(SCARD_E_INVALID_VALUE);
    }

    *pdwActiveProtocol = allowedProtocol;

    return SCARD_S_SUCCESS;
  }

  DWORD Smartcard::disconnect(DWORD dwDisposition) {
    disposition = dwDisposition;
    return SCARD_S_SUCCESS;
  }

  DWORD Smartcard::beginTransaction() {
    if (transaction) {
      return static_cast<DWORD>(SCARD_E_SHARING_VIOLATION);
    }
    transaction = true;
    return SCARD_S_SUCCESS;
  }

  DWORD Smartcard::endTransaction(DWORD dwDisposition) {
    if (transaction) {
      disposition = dwDisposition;
      transaction = false;
      if (dwDisposition == SCARD_RESET_CARD) {
        return static_cast<DWORD>(SCARD_W_RESET_CARD);
      }
      return SCARD_S_SUCCESS;
    }

    return static_cast<DWORD>(SCARD_E_NOT_TRANSACTED);
  }

  DWORD Smartcard::getPreferredProtocol() {
    return allowedProtocol;
  }

  const vector<unsigned char> Smartcard::getATR() {
    return ATR;
  }

  unique_ptr<Smartcard> Smartcard::instance_of(const string &impl) {
    if (impl == "test") {
      return make_unique<TestSmartcard>();
    }

    return nullptr;
  }

}
