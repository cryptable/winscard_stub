/**
 * Implementation of the winscard stubbing interface
 */
#include <wintypes.h>
#include <winscard.h>
#include <memory.h>
#include <vector>
#include <memory>
#include <future>
#include <unordered_map>
#include <pcsclite.h>
#include "stubbing.h"
#include "winscard_stub.h"
#include "missing_stl.h"

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

class WinsCardEvent {
public:
  virtual DWORD getReaderState(SCARD_READERSTATE readerState[], DWORD cReaders) = 0;
};

/**
 * Smartcard virtual simulator base class. The specific implemenations wlll have to override the execute
 * function
 */
class SmartCard {
public:
  /**
   * Constructor which will define the behavior of sharing mode and the supported protocols
   * Must be called by the derived class
   * @param dwSharingMode
   * @param dwProtocol
   */
  explicit SmartCard(const vector<unsigned  char> &atr, DWORD dwSharingMode = SCARD_SHARE_SHARED, DWORD dwProtocol = SCARD_PROTOCOL_T0) :
    allowedSharingModes(dwSharingMode),
    allowedProtocol (dwProtocol),
    disposition(SCARD_LEAVE_CARD),
    ATR(atr) {
  };

  /**
   * Connect to the smartcard, which will verify the supported sharingMode and protocols. The connect function returns
   * a handle to the smartcard and the active protocol.
   *
   * @param dwShareMode
   * @param dwPreferredProtocols
   * @param phCard
   * @param pdwActiveProtocol
   * @return SCARD_S_SUCCESS, SCARD_E_INVALID_VALUE
   */
  DWORD connect(DWORD dwShareMode, DWORD dwPreferredProtocols, LPSCARDHANDLE phCard, LPDWORD pdwActiveProtocol) {

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
  };

  /**
   * Disconnect from the smartcard with extra action to take for the card during the disconnect
   *
   * @param handle The handle of the Smartcard to disconnect
   * @param disposition action to take of the smartcard
   * @return SCARD_S_SUCCESS, SCARD_E_INVALID_HANDLE
   */
  DWORD disconnect(SCARDHANDLE handle, DWORD dwDisposition) {
    if (scardHandles.erase(handle) == 0) {
      return static_cast<DWORD>(SCARD_E_INVALID_HANDLE);
    }
    disposition = dwDisposition;
    return SCARD_S_SUCCESS;
  }

  /**
   * Begin multiple calls to the Smartcard
   * @param handle The handle to the smartcard
   * @return SCARD_S_SUCCESS, SCARD_E_SHARING_VIOLATION, SCARD_E_INVALID_HANDLE
   */
  DWORD beginTransaction(SCARDHANDLE handle) {
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

  /**
   * End multiple calls to the Smartcard
   * @param handle The handle to the smartcard
   * @return SCARD_S_SUCCESS, SCARD_E_SHARING_VIOLATION, SCARD_E_INVALID_HANDLE
   */
  DWORD endTransaction(SCARDHANDLE handle, DWORD dwDisposition) {
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

  /**
   * Function to override by the specific implemented Smartcard.
   *
   * @param scardhandle handle to the smartcard
   * @param in_apdu APDU command
   * @param in_apdu_lg length of the APDU command
   * @param out_apdu APDU result
   * @param out_apd_lg length of the APDU result
   * @return SCARD_S_SUCCESS
   */
  virtual DWORD execute(SCARDHANDLE handle, const char *in_apdu, size_t in_apdu_lg, char **out_apdu, size_t *out_apd_lg) = 0;

  DWORD getPreferredProtocol() {
    return allowedProtocol;
  }
  /**
   * Function to override which returns the ATR pointer
   */
  const vector<unsigned char> getATR() {
    return ATR;
  };

  /**
   * Static member function for the factory method to instantiate the supported Smartcards.
   * @param impl
   * @return smart pointer to the SmartCard object
   */
  static unique_ptr<SmartCard> instance_of(const string &impl);

private:

  struct SmartCardContext {
    DWORD sharingMode;
    DWORD protocol;
    bool  transaction;
  };

  SCARDHANDLE scardHandleIndex{0};
  unordered_map<SCARDHANDLE, unique_ptr<SmartCardContext>> scardHandles;
  DWORD allowedSharingModes;
  DWORD allowedProtocol;
  DWORD disposition;
  vector<unsigned char> ATR;
};

class ReaderEventSubject;
class SmartcardEventSubject;

/**
 * Smartcard reader simulator: This class will be the base class for the reader implementation. In a reader only 1 card
 * can be inserted. All calls from the winscard interface will  be proxied through the reader implementation.
 */
class SmartCardReader {

public:

  SmartCardReader() = delete;

  SmartCardReader(SmartCardReader &other) = delete;

  SmartCardReader &operator=(SmartCardReader &other) = delete;

  SmartCardReader(SmartCardReader &&other) = delete;

  SmartCardReader &operator=(SmartCardReader &&other) = delete;

  ~SmartCardReader() = default;

  /**
   * Constructor of Smartcard to be called by the derived class
   *
   * @param readerName
   */
  explicit SmartCardReader(string readerName): name(std::move(readerName)), smartCard(nullptr), events(0), id(0),
                                               readerEvents(this),
                                               smartcardEvents(this) {

  };

  /**
   * Returns the default name of the reader and will be used by 'winscard' create a unique reader name
   *
   * @return
   */
  string getName() const {
    return name;
  };

  /**
   * Proxy function to connect to the Smartcard all parameters are send to the smartcard.
   *
   * @param dwShareMode
   * @param dwPreferredProtocols
   * @param phCard
   * @param pdwActiveProtocol
   * @return SCARD_S_SUCCESS, SCARD_E_NO_SMARTCARD
   */
  DWORD connectToSmartCard(DWORD dwShareMode, DWORD dwPreferredProtocols, LPSCARDHANDLE phCard, LPDWORD pdwActiveProtocol) {
    if (smartCard == nullptr) {
      return static_cast<DWORD>(SCARD_E_NO_SMARTCARD);
    }
    return smartCard->connect(dwShareMode, dwPreferredProtocols, phCard, pdwActiveProtocol);
  };

  /**
   * Proxy function to disconnect from the Smartcard all parameters are send to the smartcard.
   *
   * @param dwShareMode (see SmartCard base class)
   * @param dwPreferredProtocols (see SmartCard base class)
   * @param phCard (see SmartCard base class)
   * @param pdwActiveProtocol (see SmartCard base class)
   * @return SCARD_S_SUCCESS, SCARD_E_NO_SMARTCARD
   */
  DWORD disconnectFromSmartCard(SCARDHANDLE hCard, DWORD dwDisposition) {
    if (smartCard == nullptr) {
      return static_cast<DWORD>(SCARD_E_NO_SMARTCARD);
    }
    int ret = smartCard->disconnect(hCard, dwDisposition);
    if (dwDisposition == SCARD_EJECT_CARD) {
      ejectCard();
    }
    return ret;
  };

  /**
   * Function which inserts a Smartcard into the reader
   *
   * @param card name of one of the implemented cards ("test")
   * @return SCARD_S_SUCCESS, SCARD_E_CARD_IN_READER, SCARD_E_CARD_UNSUPPORTED
   */
  DWORD insertCard(const string &card) {
    if (nullptr != smartCard) {
      return static_cast<DWORD>(SCARD_E_CARD_IN_READER);
    }
    smartCard = SmartCard::instance_of(card);
    if (nullptr == smartCard) {
      return static_cast<DWORD>(SCARD_E_CARD_UNSUPPORTED);
    }
    events++;
    smartcardEvents.notify();

    return SCARD_S_SUCCESS;
  }

  /**
   * Function which removes a Smartcard from the reader
   *
   * @param card name of one of the implemented cards ("test")
   * @return SCARD_S_SUCCESS, SCARD_E_CARD_IN_READER, SCARD_E_CARD_UNSUPPORTED
   */
  DWORD ejectCard(void) {
    if (nullptr == smartCard) {
      return static_cast<DWORD>(SCARD_E_NO_SMARTCARD);
    }
    smartCard.reset(nullptr);
    events++;
    smartcardEvents.notify();
    return SCARD_S_SUCCESS;
  }

  /**
   * Proxy begin transaction to the smartcard
   *
   * @param scardhandle handle to the scmartcard
   * @return SCARD_S_SUCCESS, SCARD_E_NO_SMARTCARD + errors of Smartcard
   */
  DWORD beginTransactionOnSmartcard(SCARDHANDLE scardhandle) {
    if (smartCard == nullptr) {
      return static_cast<DWORD>(SCARD_E_NO_SMARTCARD);
    }
    return smartCard->beginTransaction(scardhandle);
  }

  /**
   * Proxy end transaction to the smartcard if SCARD_EJECT_CARD is send as dwDisposition, the smartcard is removed
   *
   * @param scardhandle handle to the scmartcard
   * @return SCARD_S_SUCCESS, SCARD_E_NO_SMARTCARD + errors of Smartcard
   */
  DWORD endTransactionOnSmartcard(SCARDHANDLE scardhandle, DWORD dwDisposition) {
    if (smartCard == nullptr) {
      return static_cast<DWORD>(SCARD_E_NO_SMARTCARD);
    }
    DWORD ret = smartCard->endTransaction(scardhandle, dwDisposition);
    if (dwDisposition == SCARD_EJECT_CARD) {
      ejectCard();
    }
    return ret;
  }

  DWORD smartcardStatus(SCARDHANDLE hCard, LPSTR mszReaderName, LPDWORD pcchReaderLen, LPDWORD pdwState, LPDWORD pdwProtocol, LPBYTE pbAtr, LPDWORD pcbAtrLen) {
    (void)hCard;

    *pdwState = getState();

    if (smartCard == nullptr){
      return SCARD_W_REMOVED_CARD;
    }
    *pdwProtocol = smartCard->getPreferredProtocol();

    if (*pcchReaderLen < (name.size() + 1)) {
      *pcbAtrLen = smartCard->getATR().size();
      *pcchReaderLen = name.size() + 1;
      return SCARD_E_INSUFFICIENT_BUFFER;
    }
    *pcchReaderLen = name.size() + 1;
    if (mszReaderName != nullptr) {
      strncpy(mszReaderName, name.c_str(), name.size());
      mszReaderName[name.size()] = '\0';
    }
    if (*pcbAtrLen < smartCard->getATR().size()) {
      *pcbAtrLen = smartCard->getATR().size();
      return SCARD_E_INSUFFICIENT_BUFFER;
    }
    *pcbAtrLen = smartCard->getATR().size();
    if (pbAtr != nullptr) {
      int index = 0;
      for (auto c : smartCard->getATR()) {
        pbAtr[index++] = c;
      }
    }

    return SCARD_S_SUCCESS;
  }

  /** !
   * The function which must be overwritten by the implemented reader, which should be mainly a pinpad
   * or non-pinpad reader. It will -most of the time- be a proxy function to the smartcard.
   *
   * @param scardhandle handle to the smartcard
   * @param in_apdu APDU command
   * @param in_apdu_lg length of the APDU command
   * @param out_apdu APDU result
   * @param out_apd_lg length of the APDU result
   * @return SCARD_S_SUCCESS,
   */
  virtual DWORD execute(SCARDHANDLE scardhandle, const char *in_apdu, size_t in_apdu_lg, char **out_apdu, size_t *out_apd_lg) = 0;

  /**
   * Function to instantiate a supported reader ("Non Pinpad Reader", "Pinpad Reader")
   *
   * @param impl name of the supported reader
   * @return smart pointer to the SmartCardReader object
   */
  static shared_ptr<SmartCardReader> instance_of(const string &impl);

  /**
   * SetId: number for duplicate reader
   * TODO: split into context and template
   */
  void setId(unsigned int nbr) {
    id = nbr;
    readerId = name + " " + to_string(nbr);
  }

  const string &getReaderIdentifier() {
    return readerId;
  }

  void getEventInfo(LPSCARD_READERSTATE readerState) {
    if (smartCard) {
      readerState->dwEventState = SCARD_STATE_PRESENT;
      readerState->cbAtr = smartCard->getATR().size();
      for (unsigned int i=0; i < smartCard->getATR().size(); i++) {
        readerState->rgbAtr[i] = smartCard->getATR()[i];
      }
    }
    else {
      readerState->dwEventState = SCARD_STATE_EMPTY;
    }
  }

protected:
  const string name;

private:

  DWORD getState() {
    DWORD state = 0xFFFF0000 & (events << 16);

    if (smartCard == nullptr) {
      state = state | SCARD_ABSENT;
    }

    if (smartCard != nullptr) {
      state = state | SCARD_SPECIFIC;
    }
    return state;
  }


  unique_ptr<SmartCard> smartCard;

  unsigned int events;

  unsigned int id;

  string readerId;

  SmartcardEventSubject smartcardEvents;
};

/**
 * Non Pinpad Smartcard reader implementation
 */
class NonPinpadReader : public SmartCardReader {
public:

  /**
   * Constructor to instantiate the Non Pinpad Reader object
   */
  NonPinpadReader() : SmartCardReader("Non Pinpad Reader") {

  };

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
  DWORD execute(SCARDHANDLE scardhandle, const char *in_apdu, size_t in_apdu_lg, char **out_apdu, size_t *out_apd_lg) override {
    return static_cast<DWORD>(SCARD_E_UNSUPPORTED_FEATURE);
  }

private:
};

/**
 * Pinpad Smart card reader implementation
 */
class PinpadReader : public  SmartCardReader {
public:
  /**
   * Constructor to instantiate the Pinpad Reader object
   */
  PinpadReader(): SmartCardReader("Pinpad Reader") {

  }

  /**
   * Specific implementation of the execute command for the Pinpad Reader. Commands to the smartcard will be proxied
   * by this function to the smartcard.
   *
   * @param scardhandle handle to the smartcard
   * @param in_apdu APDU command
   * @param in_apdu_lg length of the APDU command
   * @param out_apdu APDU result
   * @param out_apd_lg length of the APDU result
   * @return SCARD_S_SUCCESS
   */
  DWORD execute(SCARDHANDLE scardhandle, const char *in_apdu, size_t in_apdu_lg, char **out_apdu, size_t *out_apd_lg) override {
    return static_cast<DWORD>(SCARD_E_UNSUPPORTED_FEATURE);
  }

};

shared_ptr<SmartCardReader> SmartCardReader::instance_of(const string &impl) {
  if (impl == "Non Pinpad Reader") {
    return make_shared<NonPinpadReader>();
  }

  if (impl == "Pinpad Reader") {
    return make_shared<PinpadReader>();
  }

  return nullptr;
}

/**
 * Implementation of a test smartcard
 */
class TestSmartCard : public SmartCard {
public:
  TestSmartCard() : SmartCard( vector<unsigned char>({ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16 }) ) {

  };

  DWORD execute(SCARDHANDLE handle, const char *in_apdu, size_t in_apdu_lg, char **out_apdu, size_t *out_apd_lg) override {
    return static_cast<DWORD>(SCARD_E_UNSUPPORTED_FEATURE);
  }

};

unique_ptr<SmartCard> SmartCard::instance_of(const string &impl) {
  if (impl == "test") {
    return make_unique<TestSmartCard>();
  }

  return nullptr;
}

/**
 * Event Handler
 */
class WinscardEventObserver;



class ReaderEventSubject : public WinscardEventSubject {

public:
  explicit ReaderEventSubject(shared_ptr<SmartCardReader> rdr) : reader(rdr) { };

  virtual DWORD getReaderState(LPSCARD_READERSTATE readerState) {
    if (string(readerState->szReader) == "\\\\?PnP?\\Notification") {
      readerState->szReader = reader->getReaderIdentifier().c_str();
      readerState->dwEventState = SCARD_STATE_CHANGED;
      return SCARD_S_SUCCESS;
    }
    return SCARD_E_UNKNOWN_READER;
  }

private:
  shared_ptr<SmartCardReader> reader;
};

class SmartcardEventSubject : public WinscardEventSubject {

public:
  explicit SmartcardEventSubject(shared_ptr<SmartCardReader> rdr) : reader(rdr) { };

  virtual DWORD getReaderState(LPSCARD_READERSTATE readerState) {
    if (string(readerState->szReader) == reader->getReaderIdentifier()) {
      readerState->szReader = reader->getReaderIdentifier().c_str();
      reader->getEventInfo(readerState);
      return SCARD_S_SUCCESS;
    }
    return SCARD_E_UNKNOWN_READER;
  }

private:
  shared_ptr<SmartCardReader> reader;
};

/**
 * Context to the winscard subsystem
 */
class WinscardContext {

public:

  WinscardContext() = default;

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

  DWORD attachReader(string new_reader) {
    unsigned int next = 0;

    auto new_reader_impl = SmartCardReader::instance_of(new_reader);

    if (new_reader_impl == nullptr)
      return SCARD_E_UNKNOWN_READER;

    for (auto reader : readers) {
      if (reader.second->getName() == new_reader_impl->getName()) {
        next++;
      }
    }
    new_reader_impl->setId(next);
    readers[new_reader_impl->getReaderIdentifier()] = new_reader_impl;

    readerEvents.notify();

    refreshReaderNames();

    return SCARD_S_SUCCESS;
  }

  DWORD insertSmartCardIn(const string &reader, const string &card) {
    try {
      DWORD ret = readers.at(reader)->insertCard(card);

      return ret;
    }
    catch (out_of_range &oor) {
      return SCARD_E_READER_UNAVAILABLE;
    }
  }

  DWORD removeSmartCardFrom(const string &reader) {
    try {
      DWORD ret = readers.at(reader)->ejectCard();

      return ret;
    }
    catch (out_of_range &oor) {
      return SCARD_E_READER_UNAVAILABLE;
    }
  }

  DWORD connectToSmartCard(const char *readerName, DWORD dwShareMode, DWORD dwPreferredProtocols, LPSCARDHANDLE phCard, LPDWORD pdwActiveProtocol) {
    DWORD ret = 0;

    try {
      SCARDHANDLE hCard = 0;
      ret = readers.at(readerName)->connectToSmartCard(dwShareMode, dwPreferredProtocols, &hCard, pdwActiveProtocol);
      if (ret == SCARD_S_SUCCESS) {
        smartcards_ctx[++cardctx] = make_unique<struct scard_ctx>(readers[readerName], hCard);
        *phCard = cardctx;
      }
      return ret;
    }
    catch (out_of_range &oor) {
      return static_cast<DWORD>(SCARD_E_UNKNOWN_READER);
    }
  }

  DWORD disconnectFromSmartCard(SCARDHANDLE hCard, DWORD dwDisposition) {

    try {
      DWORD ret = smartcards_ctx.at(hCard)->reader->disconnectFromSmartCard(smartcards_ctx.at(hCard)->scardhandle, dwDisposition);
      smartcards_ctx.erase(hCard);
      return ret;
    }
    catch (out_of_range &oor) {
      return static_cast<DWORD>(SCARD_E_INVALID_HANDLE);
    }
  }

  DWORD beginTransactionOnSmartcard(SCARDHANDLE hCard) {

    try {
      DWORD ret = smartcards_ctx.at(hCard)->reader->beginTransactionOnSmartcard(smartcards_ctx.at(hCard)->scardhandle);
      return ret;
    }
    catch (out_of_range &oor) {
      return static_cast<DWORD>(SCARD_E_INVALID_HANDLE);
    }
  }

  DWORD endTransactionOnSmartcard(SCARDHANDLE hCard, DWORD dwDisposition) {

    try {
      DWORD ret = smartcards_ctx.at(hCard)
        ->reader
        ->endTransactionOnSmartcard(smartcards_ctx.at(hCard)->scardhandle,dwDisposition);
      return ret;
    }
    catch (out_of_range &oor) {
      return static_cast<DWORD>(SCARD_E_INVALID_HANDLE);
    }
  }

  DWORD smartcardStatus(SCARDHANDLE hCard, LPSTR mszReaderName, LPDWORD pcchReaderLen, LPDWORD pdwState, LPDWORD pdwProtocol, LPBYTE pbAtr, LPDWORD pcbAtrLen) {
    try {
     return smartcards_ctx.at(hCard)->reader->smartcardStatus(hCard, mszReaderName, pcchReaderLen, pdwState, pdwProtocol, pbAtr, pcbAtrLen);
    }
    catch (out_of_range &oor) {
      return static_cast<DWORD>(SCARD_E_INVALID_HANDLE);
    }
  }

  // TODO: No support for multithreaded SCardGetStatusChange! Need a vector of promises or condition variables
  DWORD contextGetStatusChange(DWORD dwTimeout, SCARD_READERSTATE *rgReaderStates, DWORD cReaders) {
    {
      std::mutex m;
      shared_ptr<WinscardEventObserver> observer = make_shared<WinscardEventObserver>(rgReaderStates, cReaders);
      readerEvents.attach(observer);
      observer->wait_for(m, dwTimeout);
    }

    return SCARD_E_UNEXPECTED;
  }

private:

  // Support for Smartcard handles versus reader
  struct scard_ctx {
    shared_ptr<SmartCardReader> reader;
    SCARDHANDLE scardhandle;
  };

  unordered_map<SCARDHANDLE, unique_ptr<struct scard_ctx>> smartcards_ctx;
  SCARDHANDLE cardctx = 0;

  // Readers
  unordered_map<string, shared_ptr<SmartCardReader>> readers;

  unsigned char *readerNames = nullptr;
  size_t readerNamesLg = 0;

  void refreshReaderNames() {

    delete[] readerNames;
    readerNamesLg = 0;

    for (auto &reader : readers) {
      readerNamesLg += reader.first.size() + 1;
    }
    readerNamesLg += 1;
    readerNames = new unsigned char[readerNamesLg];
    memset((void *)readerNames, 0, readerNamesLg);
    unsigned int index = 0;
    for (auto &reader : readers) {
      memcpy((void *)&(readerNames[index]), reader.first.c_str(), reader.first.size());
      index += reader.first.size();
      index++;
    }
  }

  ReaderEventSubject readerEvents;
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

PCSC_API LONG SCardAttachReader(SCARDCONTEXT hContext, LPCSTR szReader)
{
  try {
    return g_contexts.at(hContext)->attachReader(szReader);
  }
  catch (out_of_range &oor) {
    return SCARD_E_INVALID_HANDLE;
  }
}

PCSC_API LONG SCardInsertSmartCardInReader(SCARDCONTEXT hContext, LPCSTR szReader, LPCSTR szCard)
{
  try {
    return g_contexts.at(hContext)->insertSmartCardIn(szReader, szCard);
  }
  catch (out_of_range &oor) {
    return SCARD_E_INVALID_HANDLE;
  }
}

PCSC_API LONG SCardRemoveSmartCardFromReader(SCARDCONTEXT hContext, LPCSTR szReader) {
  try {
    return g_contexts.at(hContext)->removeSmartCardFrom(szReader);
  }
  catch (out_of_range &oor) {
    return SCARD_E_INVALID_HANDLE;
  }
}


PCSC_API LONG SCardEstablishContext(DWORD dwScope, LPCVOID pvReserved1, LPCVOID pvReserved2, LPSCARDCONTEXT phContext)
{
  (void *)pvReserved1;
  (void *)pvReserved2;

  if (phContext == nullptr) {
    return get_return_code_for("winscard", __FUNCTION_NAME__, SCARD_E_INVALID_PARAMETER);
  }
  if ((dwScope != SCARD_SCOPE_USER)
      && (dwScope != SCARD_SCOPE_TERMINAL)
         && (dwScope != SCARD_SCOPE_SYSTEM)) {
    return get_return_code_for("winscard", __FUNCTION_NAME__, SCARD_E_INVALID_VALUE);
  }
  // Default behavior
  *phContext = g_context_index;
  g_contexts[g_context_index] = make_shared<WinscardContext>();
  g_context_index++;

  // Stubbed behavior
  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardReleaseContext(SCARDCONTEXT hContext)
{
  if (g_contexts.erase(hContext) == 0) {
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
    g_cardhandles[g_handle_index] = make_unique<struct g_card_handle>(shared_ptr<WinscardContext>(g_contexts[hContext]), hCard);
    g_handle_index++;
  }
  catch (out_of_range &oor) {
    return get_return_code_for("winscard", __FUNCTION_NAME__, SCARD_E_INVALID_HANDLE);
  }

  return get_return_code_for("winscard", __FUNCTION_NAME__ , default_return);
}

PCSC_API LONG SCardReconnect(SCARDHANDLE hCard, DWORD dwShareMode, DWORD dwPreferredProtocols, DWORD dwInitialization, LPDWORD pdwActiveProtocol)
{
  // TODO: Implementation necessary
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

  return get_return_code_for("winscard", __FUNCTION_NAME__ , default_return);
}

PCSC_API LONG SCardBeginTransaction(SCARDHANDLE hCard)
{
  DWORD default_return = 0;
  try {
    default_return = g_cardhandles.at(hCard)->winscard_ctx->beginTransactionOnSmartcard(g_cardhandles.at(hCard)->local_cardhandle);
  }
  catch (out_of_range &oor) {
    return get_return_code_for("winscard", __FUNCTION_NAME__, SCARD_E_INVALID_HANDLE);
  }

  return get_return_code_for("winscard", __FUNCTION_NAME__ , default_return);
}

PCSC_API LONG SCardEndTransaction(SCARDHANDLE hCard, DWORD dwDisposition)
{
  DWORD default_return = 0;
  try {
    default_return = g_cardhandles.at(hCard)->winscard_ctx->endTransactionOnSmartcard(g_cardhandles.at(hCard)->local_cardhandle, dwDisposition);
  }
  catch (out_of_range &oor) {
    return get_return_code_for("winscard", __FUNCTION_NAME__, SCARD_E_INVALID_HANDLE);
  }

  return get_return_code_for("winscard", __FUNCTION_NAME__ , default_return);
}

PCSC_API LONG SCardStatus(SCARDHANDLE hCard, LPSTR mszReaderName, LPDWORD pcchReaderLen, LPDWORD pdwState, LPDWORD pdwProtocol, LPBYTE pbAtr, LPDWORD pcbAtrLen)
{
  DWORD default_return = 0;
  try {
    default_return = g_cardhandles.at(hCard)
      ->winscard_ctx
      ->smartcardStatus(g_cardhandles.at(hCard)->local_cardhandle, mszReaderName, pcchReaderLen,
                        pdwState, pdwProtocol,
                        pbAtr, pcbAtrLen);
  }
  catch (out_of_range &oor) {
    return get_return_code_for("winscard", __FUNCTION_NAME__, SCARD_E_INVALID_HANDLE);
  }
  return get_return_code_for("winscard", __FUNCTION_NAME__ , default_return);
}

PCSC_API LONG SCardGetStatusChange(SCARDCONTEXT hContext, DWORD dwTimeout, SCARD_READERSTATE *rgReaderStates, DWORD cReaders)
{
  DWORD default_return = 0;
  try {
    default_return = g_contexts.at(hContext)->contextGetStatusChange(dwTimeout, rgReaderStates, cReaders);
  }
  catch (out_of_range &oor) {
    return get_return_code_for("winscard", __FUNCTION_NAME__, SCARD_E_INVALID_HANDLE);
  }

  return get_return_code_for("winscard", __FUNCTION_NAME__ , default_return);
}

PCSC_API LONG SCardControl(SCARDHANDLE hCard, DWORD dwControlCode, LPCVOID pbSendBuffer, DWORD cbSendLength, LPVOID pbRecvBuffer, DWORD cbRecvLength, LPDWORD lpBytesReturned)
{
  // TODO: Implementation necessary

  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardTransmit(SCARDHANDLE hCard, const SCARD_IO_REQUEST *pioSendPci, LPCBYTE pbSendBuffer, DWORD cbSendLength, SCARD_IO_REQUEST *pioRecvPci, LPBYTE pbRecvBuffer, LPDWORD pcbRecvLength)
{
  // TODO: Implementation necessary

  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardListReaderGroups(SCARDCONTEXT hContext, LPSTR mszGroups, LPDWORD pcchGroups)
{
  // TODO: Implementation necessary

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
      if ((data == nullptr)
          || (data_lg == 0)) {
        // clear output variables
        if ((mszReaders != nullptr) && (pcchReaders != nullptr)) {
          memset(mszReaders, '\0', *pcchReaders);
        }
        if (pcchReaders != nullptr) {
          *pcchReaders = 0;
        }
        return get_return_code_for("winscard", __FUNCTION_NAME__, SCARD_E_NO_READERS_AVAILABLE);
      }
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
  // TODO: Implementation necessary

  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardCancel(SCARDCONTEXT hContext)
{
  // TODO: Implementation necessary

  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardGetAttrib(SCARDHANDLE hCard, DWORD dwAttrId, LPBYTE pbAttr, LPDWORD pcbAttrLen)
{
  // TODO: Implementation necessary

  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardSetAttrib(SCARDHANDLE hCard, DWORD dwAttrId, LPCBYTE pbAttr, DWORD cbAttrLen)
{
  // TODO: Implementation necessary

  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}
