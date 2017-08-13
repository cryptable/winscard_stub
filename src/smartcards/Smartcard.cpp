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
    ATR(atr) {
  }

  DWORD Smartcard::connect(DWORD dwShareMode, DWORD dwPreferredProtocols, LPSCARDHANDLE phCard, LPDWORD pdwActiveProtocol) {

    if (dwShareMode != allowedSharingModes) {
      return static_cast<DWORD>(SCARD_E_INVALID_VALUE);
    }
    if ((dwPreferredProtocols & allowedProtocol) != allowedProtocol) {
      return static_cast<DWORD>(SCARD_E_INVALID_VALUE);
    }

    scardHandles[++scardHandleIndex] = make_unique<struct SmartCardContext>(dwShareMode, allowedProtocol, false);
    *phCard = scardHandleIndex;
    *pdwActiveProtocol = allowedProtocol;

    return SCARD_S_SUCCESS;
  }

  DWORD Smartcard::disconnect(SCARDHANDLE handle, DWORD dwDisposition) {
    if (scardHandles.erase(handle) == 0) {
      return static_cast<DWORD>(SCARD_E_INVALID_HANDLE);
    }
    disposition = dwDisposition;
    return SCARD_S_SUCCESS;
  }

  DWORD Smartcard::beginTransaction(SCARDHANDLE handle) {
    try {
      if (scardHandles.at(handle)->transaction)
        return SCARD_E_SHARING_VIOLATION;

      scardHandles.at(handle)->transaction = true;
      return SCARD_S_SUCCESS;
    }
    catch (out_of_range &oor) {
      return SCARD_E_INVALID_HANDLE;
    }
  }

  DWORD Smartcard::endTransaction(SCARDHANDLE handle, DWORD dwDisposition) {
    try {
      if (!scardHandles.at(handle)->transaction)
        return SCARD_E_NOT_TRANSACTED;

      scardHandles.at(handle)->transaction = false;
      disposition = dwDisposition;

      if (disposition == SCARD_RESET_CARD)
        return SCARD_W_RESET_CARD;

      return SCARD_S_SUCCESS;
    }
    catch (out_of_range &oor) {
      return SCARD_E_INVALID_HANDLE;
    }
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
