/**
 * Implementation of the winscard stubbing interface
 */
#include "stubbing.h"
#include <wintypes.h>
#include <winscard.h>
#include <cstring>
#include <vector>

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

  explicit SmartCardReader(string readerName);;

  string getName() const {
    return name;
  };

  virtual void execute(const char *in_apdu, size_t in_apdu_lg, char **out_apdu, size_t *out_apd_lg) = 0;

protected:
  const string name;

private:
  unsigned int winscard_state;
};

SmartCardReader::SmartCardReader(string readerName) : name(std::move(readerName)),
                                                      winscard_state(SMARTCARD_READER_NOT_CONNECTED) {

}

/**
 * Smartcard simulator
 */
class SmartCard {

private:
};

/**
 * Normal Smart card reader
 */
class NonPinpadReader : public SmartCardReader {
public:
  NonPinpadReader() : SmartCardReader("Non Pinpad Reader") {

  };

  void execute(const char *in_apdu, size_t in_apdu_lg, char **out_apdu, size_t *out_apd_lg) override {

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

  void execute(const char *in_apdu, size_t in_apdu_lg, char **out_apdu, size_t *out_apd_lg) override {

  }

private:

};

/**
 * Context to the winscard subsystem
 */
class WinscardContext {

private:
};

/**
 * Global configuration for the winscard stub
 */


/**
 * Winscard handles
 */
unsigned int g_index = 0;
vector<WinscardContext *> g_contexts;

PCSC_API LONG SCardEstablishContext(DWORD dwScope, LPCVOID pvReserved1, LPCVOID pvReserved2, LPSCARDCONTEXT phContext)
{
  // Default behavior
  *phContext = g_index;
  g_contexts.push_back(new WinscardContext());
  g_index++;

  // Stubbed behavior
  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardReleaseContext(SCARDCONTEXT hContext)
{
  // Default behavior
  if (hContext < g_index) {
    if (g_contexts[hContext] != NULL) {
      delete g_contexts[hContext];
      g_contexts[hContext] = NULL;
    }
  }

  // Stubbed behavior
  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardIsValidContext(SCARDCONTEXT hContext)
{

  if (hContext >= g_index) {
    return SCARD_E_INVALID_HANDLE;
  }
  if (g_contexts[hContext] == NULL) {
    return SCARD_E_INVALID_HANDLE;
  }

  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardConnect(SCARDCONTEXT hContext, LPCSTR szReader, DWORD dwShareMode, DWORD dwPreferredProtocols, LPSCARDHANDLE phCard, LPDWORD pdwActiveProtocol)
{

  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardReconnect(SCARDHANDLE hCard, DWORD dwShareMode, DWORD dwPreferredProtocols, DWORD dwInitialization, LPDWORD pdwActiveProtocol)
{

  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardDisconnect(SCARDHANDLE hCard, DWORD dwDisposition)
{

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

  return get_return_code_for("winscard", __FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardListReaders(SCARDCONTEXT hContext, LPCSTR mszGroups, LPSTR mszReaders, LPDWORD pcchReaders)
{
  const unsigned char *data = NULL;
  unsigned long data_lg = 0;

  // Overwritten function
  if (get_out_parameter_for("winscard", __FUNCTION_NAME__, "mszReaders", &data, &data_lg)) {
    if (pcchReaders) {
      *pcchReaders = data_lg;
    }
    if (mszReaders) {
      memcpy(mszReaders, data, *pcchReaders);
    }
  }
  else {
    // Default behavior

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
