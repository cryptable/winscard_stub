//
// Created by david on 8/11/17.
//

#include <cstring>
#include <winscard_stub.h>
#include "SmartcardReader.h"
#include "PinpadReader.h"
#include "NonPinpadReader.h"

using namespace std;
using namespace smartcards;

namespace readers {

  SmartcardReader::SmartcardReader(string readerName): name(move(readerName)), smartcard(nullptr), events(0), id(0) {

  };

  string SmartcardReader::getName() const {
    return name;
  };

  DWORD SmartcardReader::connectToSmartCard(DWORD dwShareMode, DWORD dwPreferredProtocols, LPSCARDHANDLE phCard, LPDWORD pdwActiveProtocol) {
    if (smartcard == nullptr) {
      return static_cast<DWORD>(SCARD_E_NO_SMARTCARD);
    }
    return smartcard->connect(dwShareMode, dwPreferredProtocols, phCard, pdwActiveProtocol);
  };

  DWORD SmartcardReader::disconnectFromSmartCard(SCARDHANDLE hCard, DWORD dwDisposition) {
    if (smartcard == nullptr) {
      return static_cast<DWORD>(SCARD_E_NO_SMARTCARD);
    }
    int ret = smartcard->disconnect(hCard, dwDisposition);
    if (dwDisposition == SCARD_EJECT_CARD) {
      ejectCard();
    }
    return ret;
  };

  DWORD SmartcardReader::insertCard(const string &card) {
    if (nullptr != smartcard) {
      return static_cast<DWORD>(SCARD_E_CARD_IN_READER);
    }
    smartcard = Smartcard::instance_of(card);
    if (nullptr == smartcard) {
      return static_cast<DWORD>(SCARD_E_CARD_UNSUPPORTED);
    }
    events++;
    // smartcardEvents.notify();

    return SCARD_S_SUCCESS;
  }

  DWORD SmartcardReader::ejectCard(void) {
    if (nullptr == smartcard) {
      return static_cast<DWORD>(SCARD_E_NO_SMARTCARD);
    }
    smartcard.reset(nullptr);
    events++;
    // smartcardEvents.notify();

    return SCARD_S_SUCCESS;
  }

  DWORD SmartcardReader::beginTransactionOnSmartcard(SCARDHANDLE scardhandle) {
    if (smartcard == nullptr) {
      return static_cast<DWORD>(SCARD_E_NO_SMARTCARD);
    }
    return smartcard->beginTransaction(scardhandle);
  }

  DWORD SmartcardReader::endTransactionOnSmartcard(SCARDHANDLE scardhandle, DWORD dwDisposition) {
    if (smartcard == nullptr) {
      return static_cast<DWORD>(SCARD_E_NO_SMARTCARD);
    }
    DWORD ret = smartcard->endTransaction(scardhandle, dwDisposition);
    if (dwDisposition == SCARD_EJECT_CARD) {
      ejectCard();
    }
    return ret;
  }

  DWORD SmartcardReader::smartcardStatus(SCARDHANDLE hCard, LPSTR mszReaderName, LPDWORD pcchReaderLen, LPDWORD pdwState, LPDWORD pdwProtocol, LPBYTE pbAtr, LPDWORD pcbAtrLen) {
    (void)hCard;

    *pdwState = getState();

    if (smartcard == nullptr){
      return SCARD_W_REMOVED_CARD;
    }
    *pdwProtocol = smartcard->getPreferredProtocol();

    if (*pcchReaderLen < (name.size() + 1)) {
      *pcbAtrLen = smartcard->getATR().size();
      *pcchReaderLen = name.size() + 1;
      return SCARD_E_INSUFFICIENT_BUFFER;
    }
    *pcchReaderLen = name.size() + 1;
    if (mszReaderName != nullptr) {
      strncpy(mszReaderName, name.c_str(), name.size());
      mszReaderName[name.size()] = '\0';
    }
    if (*pcbAtrLen < smartcard->getATR().size()) {
      *pcbAtrLen = smartcard->getATR().size();
      return SCARD_E_INSUFFICIENT_BUFFER;
    }
    *pcbAtrLen = smartcard->getATR().size();
    if (pbAtr != nullptr) {
      int index = 0;
      for (auto c : smartcard->getATR()) {
        pbAtr[index++] = c;
      }
    }

    return SCARD_S_SUCCESS;
  }

  void SmartcardReader::setId(unsigned int nbr) {
    id = nbr;
    readerId = name + " " + to_string(nbr);
  }

  const string &SmartcardReader::getReaderIdentifier() {
    return readerId;
  }

  void SmartcardReader::getEventInfo(LPSCARD_READERSTATE readerState) {
    if (smartcard) {
      readerState->dwEventState = SCARD_STATE_PRESENT;
      readerState->cbAtr = smartcard->getATR().size();
      for (unsigned int i=0; i < smartcard->getATR().size(); i++) {
        readerState->rgbAtr[i] = smartcard->getATR()[i];
      }
    }
    else {
      readerState->dwEventState = SCARD_STATE_EMPTY;
    }
  }

  DWORD SmartcardReader::getState() {
    DWORD state = 0xFFFF0000 & (events << 16);

    if (smartcard == nullptr) {
      state = state | SCARD_ABSENT;
    }

    if (smartcard != nullptr) {
      state = state | SCARD_SPECIFIC;
    }
    return state;
  }

  shared_ptr<SmartcardReader> SmartcardReader::instance_of(const string &impl) {
    if (impl == "Non Pinpad Reader") {
      return make_shared<NonPinpadReader>();
    }

    if (impl == "Pinpad Reader") {
      return make_shared<PinpadReader>();
    }

    return nullptr;
  }

}
