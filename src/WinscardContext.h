//
// Created by david on 8/11/17.
//

#ifndef WINSCARD_STUB_WINSCARDCONTEXT_H
#define WINSCARD_STUB_WINSCARDCONTEXT_H

#include <string>
#include <pcsclite.h>
#include "readers/SmartcardReader.h"
#include "eventhandler/ReaderEventSubject.h"

/**
 * Context to the winscard subsystem
 */
class WinscardContext {

public:

  WinscardContext();

  WinscardContext(WinscardContext &other) = delete;

  WinscardContext(WinscardContext &&other) = delete;

  WinscardContext &operator=(WinscardContext &other) = delete;

  WinscardContext &operator=(WinscardContext &&other) = delete;

  ~WinscardContext();

  void getReaderNames(const unsigned char **mszReaders, size_t *readersLg);

  DWORD attachReader(std::string new_reader);

  DWORD insertSmartCardIn(const std::string &reader, const std::string &card);

  DWORD removeSmartCardFrom(const std::string &reader);

  DWORD connectToSmartCard(const char *readerName, DWORD dwShareMode, DWORD dwPreferredProtocols, LPSCARDHANDLE phCard, LPDWORD pdwActiveProtocol);

  DWORD disconnectFromSmartCard(SCARDHANDLE hCard, DWORD dwDisposition);

  DWORD beginTransactionOnSmartcard(SCARDHANDLE hCard);

  DWORD endTransactionOnSmartcard(SCARDHANDLE hCard, DWORD dwDisposition);

  DWORD smartcardStatus(SCARDHANDLE hCard, LPSTR mszReaderName, LPDWORD pcchReaderLen, LPDWORD pdwState, LPDWORD pdwProtocol, LPBYTE pbAtr, LPDWORD pcbAtrLen);

  // TODO: No support for multithreaded SCardGetStatusChange! Need a vector of promises or condition variables
  DWORD contextGetStatusChange(DWORD dwTimeout, SCARD_READERSTATE *rgReaderStates, DWORD cReaders);
private:

  // Support for Smartcard handles versus reader
  struct scard_ctx {
    std::shared_ptr<readers::SmartcardReader> reader;
    SCARDHANDLE scardhandle;
  };

  std::unordered_map<SCARDHANDLE, std::unique_ptr<struct scard_ctx>> smartcards_ctx;
  SCARDHANDLE cardctx = 0;

  // Readers
  std::unordered_map<std::string, std::shared_ptr<readers::SmartcardReader>> readers;

  unsigned char *readerNames = nullptr;
  size_t readerNamesLg = 0;

  void refreshReaderNames();
//  eventhandler::ReaderEventSubject readerEvents;
};


#endif //WINSCARD_STUB_WINSCARDCONTEXT_H
