/**
 * Implementation of the winscard stubbing interface
 */
#include "stubbing.h"
#include <wintypes.h>
#include <winscard.h>
#include <cstring>
#include <vector>
#include <memory>
#include <map>
#include <unordered_map>
#include <list>
#include <unordered_set>

#ifndef __FUNCTION_NAME__
  #ifdef WIN32   //WINDOWS
    #define __FUNCTION_NAME__   __FUNCTION__
  #else          //*NIX
    #define __FUNCTION_NAME__   __func__
  #endif
#endif

#define SMARTCARD_READER_NOT_CONNECTED       0
#define SMARTCARD_READER_CONNECTED           1

using namespace std;

/**
 * Smartcard simulator
 */
class SmartCard {
public:
  explicit SmartCard(DWORD dwSharingMode = SCARD_SHARE_SHARED, DWORD dwProtocol = SCARD_PROTOCOL_T0) :
    sharingMode(dwSharingMode), protocol(dwProtocol), scardHandle(0) {

  };

  DWORD connect(DWORD dwShareMode, DWORD dwPreferredProtocols, LPSCARDHANDLE phCard, LPDWORD pdwActiveProtocol) {

    if (dwShareMode != sharingMode) {
      return SCARD_E_INVALID_VALUE;
    }
    if ((dwPreferredProtocols & protocol) != protocol) {
      return SCARD_E_INVALID_VALUE;
    }

    scardHandles[++scardHandle] = 0;
    *phCard = scardHandle;
    *pdwActiveProtocol = protocol;
    return SCARD_S_SUCCESS;
  };

  DWORD disconnect(SCARDHANDLE handle, DWORD disposition) {
    if (scardHandles.erase(handle) == 0) {
      return SCARD_E_INVALID_HANDLE;
    }

    return SCARD_S_SUCCESS;
  }

  virtual DWORD execute(const char *in_apdu, size_t in_apdu_lg, char **out_apdu, size_t *out_apd_lg) = 0;

  /**
   * Static member function for the factory method
   * @param impl
   * @return
   */
  static unique_ptr<SmartCard> instance_of(const string &impl);

private:
  SCARDHANDLE scardHandle;
  unordered_map<SCARDHANDLE, int> scardHandles;
  DWORD sharingMode;
  DWORD protocol;
};

/**
 * Smartcard reader simulator
 */
class SmartCardReader {

public:

  SmartCardReader() = delete;

  SmartCardReader(SmartCardReader &other) = delete;

  SmartCardReader &operator=(SmartCardReader &other) = delete;

  SmartCardReader(SmartCardReader &&other) = delete;

  SmartCardReader &operator=(SmartCardReader &&other) = delete;

  ~SmartCardReader() = default;

  explicit SmartCardReader(string readerName): name(readerName), smartCard(nullptr) {

  };

  string getName() const {
    return name;
  };

  DWORD connectToSmartCard(DWORD dwShareMode, DWORD dwPreferredProtocols, LPSCARDHANDLE phCard, LPDWORD pdwActiveProtocol) {
    if (smartCard == nullptr) {
      return SCARD_E_NO_SMARTCARD;
    }
    return smartCard->connect(dwShareMode, dwPreferredProtocols, phCard, pdwActiveProtocol);
  };

  DWORD disconnectFromSmartCard(SCARDHANDLE hCard, DWORD dwDisposition) {
    if (smartCard == nullptr) {
      return SCARD_E_NO_SMARTCARD;
    }
    return smartCard->disconnect(hCard, dwDisposition);
  };

  void insertCard(const string card) {
    if (smartCard != nullptr) {
      throw runtime_error("Smartcard already inserted");
    }
    smartCard = SmartCard::instance_of(card);
  }

  virtual DWORD execute(const char *in_apdu, size_t in_apdu_lg, char **out_apdu, size_t *out_apd_lg) = 0;

protected:
  const string name;

private:
  unique_ptr<SmartCard> smartCard;
};

/**
 * Normal Smart card reader
 */
class NonPinpadReader : public SmartCardReader {
public:

  NonPinpadReader() : SmartCardReader("Non Pinpad Reader") {

  };

  DWORD execute(const char *in_apdu, size_t in_apdu_lg, char **out_apdu, size_t *out_apd_lg) override {
    (void)in_apdu;
  }

private:
};

/**
 * Pinpad Smart card reader
 */
class PinpadReader : public  SmartCardReader {
public:
  PinpadReader(): SmartCardReader("Pinpad Reader") {

  }

  DWORD execute(const char *in_apdu, size_t in_apdu_lg, char **out_apdu, size_t *out_apd_lg) override {

  }

};


class BeidSmartCard : public SmartCard {
public:
  BeidSmartCard() : SmartCard() {
  }

  DWORD execute(const char *in_apdu, size_t in_apdu_lg, char **out_apdu, size_t *out_apd_lg) override {
    return SCARD_E_UNSUPPORTED_FEATURE;
  }

};

unique_ptr<SmartCard> SmartCard::instance_of(const string &impl) {
  if (impl == "beid") {
    return unique_ptr<SmartCard>(new BeidSmartCard());
  }

  return nullptr;
}

/**
 * Context to the winscard subsystem
 */
class WinscardContext {

public:

  const vector<shared_ptr<SmartCardReader>> &getReaders() const {
    return readers;
  }

  WinscardContext() {
    readers.push_back(shared_ptr<PinpadReader>(new PinpadReader()));
    readers.push_back(shared_ptr<NonPinpadReader>(new NonPinpadReader()));
    refreshReaderNames();
    readers[0]->insertCard("beid");
    readers[1]->insertCard("beid");
  }

  WinscardContext(WinscardContext &other) = delete;

  WinscardContext(WinscardContext &&other) = delete;

  WinscardContext &operator=(WinscardContext &other) = delete;

  WinscardContext &operator=(WinscardContext &&other) = delete;

  ~WinscardContext() {
    delete[] readerNames;
  }

  void getReaderNames(const unsigned char **mszReaders, size_t *readersLg) {
    *readersLg = readerNamesLg;
    *mszReaders = readerNames;
  }

  DWORD connectToSmartCard(const char *readerName, DWORD dwShareMode, DWORD dwPreferredProtocols, LPSCARDHANDLE phCard, LPDWORD pdwActiveProtocol) {
    DWORD ret = 0;

    for (auto &reader : readers) {
      if (reader->getName() == readerName) {
        SCARDHANDLE hCard = 0;
        ret = reader->connectToSmartCard(dwShareMode, dwPreferredProtocols, &hCard, pdwActiveProtocol);
        if (ret == SCARD_S_SUCCESS) {
          smartcards_ctx[++cardctx] = unique_ptr<struct scard_ctx>(new (struct scard_ctx){reader, hCard});
          *phCard = cardctx;
        }
        return ret;
      }
    }
    return SCARD_E_UNKNOWN_READER;
  }

  DWORD disconnectFromSmartCard(SCARDHANDLE hCard, DWORD dwDisposition) {

    try {
      DWORD ret = smartcards_ctx.at(hCard)->reader->disconnectFromSmartCard(smartcards_ctx.at(hCard)->scardhandle, dwDisposition);
      smartcards_ctx.erase(hCard);
      return ret;
    }
    catch (out_of_range &oor) {
      return SCARD_E_INVALID_HANDLE;
    }
  }

private:
  struct scard_ctx {
    shared_ptr<SmartCardReader> reader;
    SCARDHANDLE scardhandle;
  };
  unordered_map<SCARDHANDLE, unique_ptr<struct scard_ctx>> smartcards_ctx;
  SCARDHANDLE cardctx = 0;

  vector<shared_ptr<SmartCardReader>> readers;
  unsigned char *readerNames = nullptr;
  size_t readerNamesLg = 0;

  void refreshReaderNames() {

    delete[] readerNames;

    for (auto &reader : readers) {
      readerNamesLg += reader->getName().size() + 1;
    }
    readerNamesLg += 1;
    readerNames = new unsigned char[readerNamesLg];
    memset((void *)readerNames, 0, readerNamesLg);
    unsigned int index = 0;
    for (auto &reader : readers) {
      memcpy((void *)&(readerNames[index]), reader->getName().c_str(), reader->getName().size());
      index += reader->getName().size();
      index++;
    }
  }
};

/**
 * Global configuration for the winscard stub
 */


/**
 * Winscard handles
 */

SCARDCONTEXT g_context_index = 1;
unordered_map<SCARDCONTEXT, shared_ptr<WinscardContext>> g_contexts;

SCARDHANDLE g_handle_index = 1;
struct g_card_handle {
  shared_ptr<WinscardContext> winscard_ctx;
  SCARDHANDLE local_cardhandle;
};
unordered_map<SCARDHANDLE, unique_ptr<struct g_card_handle>> g_cardhandles;

PCSC_API LONG SCardEstablishContext(DWORD dwScope, LPCVOID pvReserved1, LPCVOID pvReserved2, LPSCARDCONTEXT phContext)
{
  if (phContext == NULL) {
    return get_return_code_for("winscard", __FUNCTION_NAME__, SCARD_E_INVALID_PARAMETER);
  }
  if ((dwScope != SCARD_SCOPE_USER)
      && (dwScope != SCARD_SCOPE_TERMINAL)
         && (dwScope != SCARD_SCOPE_SYSTEM)) {
    return get_return_code_for("winscard", __FUNCTION_NAME__, SCARD_E_INVALID_VALUE);
  }
  // Default behavior
  *phContext = g_context_index;
  g_contexts[g_context_index] = shared_ptr<WinscardContext>(new WinscardContext());
  g_context_index++;

  // Stubbed behavior
  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardReleaseContext(SCARDCONTEXT hContext)
{
  if (!g_contexts.erase(hContext)) {
    return get_return_code_for("winscard", __FUNCTION_NAME__, SCARD_E_INVALID_HANDLE);
  }

  // Stubbed behavior
  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardIsValidContext(SCARDCONTEXT hContext)
{

  if (g_contexts.find(hContext) == g_contexts.end()) {
    return get_return_code_for("winscard", __FUNCTION_NAME__, SCARD_E_INVALID_HANDLE);
  }

  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardConnect(SCARDCONTEXT hContext, LPCSTR szReader, DWORD dwShareMode, DWORD dwPreferredProtocols, LPSCARDHANDLE phCard, LPDWORD pdwActiveProtocol)
{
  DWORD default_return = 0;
  SCARDHANDLE hCard = 0;

  try {
    default_return = g_contexts.at(hContext)->connectToSmartCard(szReader, dwShareMode, dwPreferredProtocols, &hCard, pdwActiveProtocol);
    *phCard = g_handle_index;
    g_cardhandles[g_handle_index] = unique_ptr<struct g_card_handle>(new (struct g_card_handle) {shared_ptr<WinscardContext>(g_contexts[hContext]), hCard});
    g_handle_index++;
  }
  catch (out_of_range &oor) {
    return get_return_code_for("winscard", __FUNCTION_NAME__, SCARD_E_INVALID_HANDLE);
  }

  return get_return_code_for("winscard", __FUNCTION_NAME__ , default_return);
}

PCSC_API LONG SCardReconnect(SCARDHANDLE hCard, DWORD dwShareMode, DWORD dwPreferredProtocols, DWORD dwInitialization, LPDWORD pdwActiveProtocol)
{

  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardDisconnect(SCARDHANDLE hCard, DWORD dwDisposition)
{
  DWORD default_return = 0;
  try {
    default_return = g_cardhandles.at(hCard)->winscard_ctx->disconnectFromSmartCard(g_cardhandles.at(hCard)->local_cardhandle, dwDisposition);
  }
  catch (out_of_range &oor) {
    return get_return_code_for("winscard", __FUNCTION_NAME__, SCARD_E_INVALID_HANDLE);
  }

  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardBeginTransaction(SCARDHANDLE hCard)
{

  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardEndTransaction(SCARDHANDLE hCard, DWORD dwDisposition)
{

  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardStatus(SCARDHANDLE hCard, LPSTR mszReaderName, LPDWORD pcchReaderLen, LPDWORD pdwState, LPDWORD pdwProtocol, LPBYTE pbAtr, LPDWORD pcbAtrLen)
{

  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardGetStatusChange(SCARDCONTEXT hContext, DWORD dwTimeout, SCARD_READERSTATE *rgReaderStates, DWORD cReaders)
{
  try {
    g_contexts.at(hContext);
  }
  catch (out_of_range &oor) {
    return get_return_code_for("winscard", __FUNCTION_NAME__, SCARD_E_INVALID_HANDLE);
  }

  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardControl(SCARDHANDLE hCard, DWORD dwControlCode, LPCVOID pbSendBuffer, DWORD cbSendLength, LPVOID pbRecvBuffer, DWORD cbRecvLength, LPDWORD lpBytesReturned)
{

  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardTransmit(SCARDHANDLE hCard, const SCARD_IO_REQUEST *pioSendPci, LPCBYTE pbSendBuffer, DWORD cbSendLength, SCARD_IO_REQUEST *pioRecvPci, LPBYTE pbRecvBuffer, LPDWORD pcbRecvLength)
{

  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardListReaderGroups(SCARDCONTEXT hContext, LPSTR mszGroups, LPDWORD pcchGroups)
{
  try {
    g_contexts.at(hContext);
  }
  catch (out_of_range &oor) {
    return get_return_code_for("winscard", __FUNCTION_NAME__, SCARD_E_INVALID_HANDLE);
  }

  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardListReaders(SCARDCONTEXT hContext, LPCSTR mszGroups, LPSTR mszReaders, LPDWORD pcchReaders)
{
  const unsigned char *data = nullptr;
  unsigned long data_lg = 0;

  if (get_out_parameter_for("winscard", __FUNCTION_NAME__, "mszReaders", &data, &data_lg) == 0) {
    // default behavior
    try {
      g_contexts.at(hContext)->getReaderNames(&data, &data_lg);
    }
    catch (out_of_range &oor) {
      // clear output variables
      if ((mszReaders != nullptr) && (pcchReaders != nullptr)) {
        memset(mszReaders, '\0', *pcchReaders);
      }
      if (pcchReaders != nullptr) {
        *pcchReaders = 0;
      }
      return get_return_code_for("winscard", __FUNCTION_NAME__, SCARD_E_INVALID_HANDLE);
    }
  }

  if ((mszReaders != nullptr)
      && (pcchReaders != nullptr)
      && (*pcchReaders) >= data_lg) {
    memcpy(mszReaders, data, *pcchReaders);
  }
  if (pcchReaders != nullptr) {
    *pcchReaders = data_lg;
  }

  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardFreeMemory(SCARDCONTEXT hContext, LPCVOID pvMem)
{
  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardCancel(SCARDCONTEXT hContext)
{
  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardGetAttrib(SCARDHANDLE hCard, DWORD dwAttrId, LPBYTE pbAttr, LPDWORD pcbAttrLen)
{
  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardSetAttrib(SCARDHANDLE hCard, DWORD dwAttrId, LPCBYTE pbAttr, DWORD cbAttrLen)
{
  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}
