/**
 * Implementation of the winscard stubbing interface
 */
#include <wintypes.h>
#include <winscard.h>
#include <memory.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include "stubbing.h"
#include "winscard_stub.h"
#include "missing_stl.h"
#include "WinscardContext.h"

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
