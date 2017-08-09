#ifndef WINSCARD_STUB_LIBRARY_H
#define WINSCARD_STUB_LIBRARY_H

#include <wintypes.h>
#include <winscard.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SCARD_E_CARD_IN_READER		((LONG)0x80100101) /**< There is already a smartcard in reader. */

/**
 * Attach a reader to the winscard stub
 * @param hContext
 * @param szReader
 * @return
 */
PCSC_API LONG SCardAttachReader(SCARDCONTEXT hContext, LPCSTR szReader);

PCSC_API LONG SCardInsertSmartCardInReader(SCARDCONTEXT hContext, LPCSTR szReader, LPCSTR szCard);

PCSC_API LONG SCardRemoveSmartCardFromReader(SCARDCONTEXT hContext, LPCSTR szReader);

#ifdef __cplusplus
};
#endif

#endif