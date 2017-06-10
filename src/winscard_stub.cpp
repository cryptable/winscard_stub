/**
 * Implementation of the winscard stubbing interface
 */
#include "winscard_stub.h"
#include "stubbing.h"
#include <wintypes.h>
#include <winscard.h>
#include <cstring>

#ifndef __FUNCTION_NAME__
  #ifdef WIN32   //WINDOWS
    #define __FUNCTION_NAME__   __FUNCTION__
  #else          //*NIX
    #define __FUNCTION_NAME__   __func__
  #endif
#endif

PCSC_API LONG SCardEstablishContext(DWORD dwScope, LPCVOID pvReserved1, LPCVOID pvReserved2, LPSCARDCONTEXT phContext)
{
  *phContext = 101;

  return get_return_code_for(__FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardReleaseContext(SCARDCONTEXT hContext)
{
  return get_return_code_for(__FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardIsValidContext(SCARDCONTEXT hContext)
{
  return SCARD_S_SUCCESS;
}

PCSC_API LONG SCardConnect(SCARDCONTEXT hContext, LPCSTR szReader, DWORD dwShareMode, DWORD dwPreferredProtocols, LPSCARDHANDLE phCard, LPDWORD pdwActiveProtocol)
{
  return SCARD_S_SUCCESS;
}

PCSC_API LONG SCardReconnect(SCARDHANDLE hCard, DWORD dwShareMode, DWORD dwPreferredProtocols, DWORD dwInitialization, LPDWORD pdwActiveProtocol)
{
  return SCARD_S_SUCCESS;
}

PCSC_API LONG SCardDisconnect(SCARDHANDLE hCard, DWORD dwDisposition)
{
  return SCARD_S_SUCCESS;
}

PCSC_API LONG SCardBeginTransaction(SCARDHANDLE hCard)
{
  return SCARD_S_SUCCESS;
}

PCSC_API LONG SCardEndTransaction(SCARDHANDLE hCard, DWORD dwDisposition)
{
  return SCARD_S_SUCCESS;
}

PCSC_API LONG SCardStatus(SCARDHANDLE hCard, LPSTR mszReaderName, LPDWORD pcchReaderLen, LPDWORD pdwState, LPDWORD pdwProtocol, LPBYTE pbAtr, LPDWORD pcbAtrLen)
{
  return SCARD_S_SUCCESS;
}

PCSC_API LONG SCardGetStatusChange(SCARDCONTEXT hContext, DWORD dwTimeout, SCARD_READERSTATE *rgReaderStates, DWORD cReaders)
{
  return SCARD_S_SUCCESS;
}

PCSC_API LONG SCardControl(SCARDHANDLE hCard, DWORD dwControlCode, LPCVOID pbSendBuffer, DWORD cbSendLength, LPVOID pbRecvBuffer, DWORD cbRecvLength, LPDWORD lpBytesReturned)
{
  return SCARD_S_SUCCESS;
}

PCSC_API LONG SCardTransmit(SCARDHANDLE hCard, const SCARD_IO_REQUEST *pioSendPci, LPCBYTE pbSendBuffer, DWORD cbSendLength, SCARD_IO_REQUEST *pioRecvPci, LPBYTE pbRecvBuffer, LPDWORD pcbRecvLength)
{
  return SCARD_S_SUCCESS;
}

PCSC_API LONG SCardListReaderGroups(SCARDCONTEXT hContext, LPSTR mszGroups, LPDWORD pcchGroups)
{
  return SCARD_S_SUCCESS;
}

PCSC_API LONG SCardListReaders(SCARDCONTEXT hContext, LPCSTR mszGroups, LPSTR mszReaders, LPDWORD pcchReaders)
{
  void *data = get_out_parameter_for(__FUNCTION_NAME__, "mszReaders");
  if (pcchReaders) {
    *pcchReaders = (DWORD)get_out_parameter_for(__FUNCTION_NAME__, "pcchReaders");
  }
  if (mszReaders) {
    memcpy(mszReaders, data, *pcchReaders);
  }

  return get_return_code_for(__FUNCTION_NAME__ ,SCARD_S_SUCCESS);
}

PCSC_API LONG SCardFreeMemory(SCARDCONTEXT hContext, LPCVOID pvMem)
{
  return SCARD_S_SUCCESS;
}

PCSC_API LONG SCardCancel(SCARDCONTEXT hContext)
{
  return SCARD_S_SUCCESS;
}

PCSC_API LONG SCardGetAttrib(SCARDHANDLE hCard, DWORD dwAttrId, LPBYTE pbAttr, LPDWORD pcbAttrLen)
{
  return SCARD_S_SUCCESS;
}

PCSC_API LONG SCardSetAttrib(SCARDHANDLE hCard, DWORD dwAttrId, LPCBYTE pbAttr, DWORD cbAttrLen)
{
  return SCARD_S_SUCCESS;
}
