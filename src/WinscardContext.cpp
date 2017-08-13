//
// Created by david on 8/11/17.
//

#include <missing_stl.h>
#include <cstring>
#include "WinscardContext.h"
using namespace std;
using namespace readers;
using namespace eventhandler;

/**
 * Context to the winscard subsystem
 */
WinscardContext::WinscardContext() {
}

WinscardContext::~WinscardContext() {
  delete[] readerNames;
}

void WinscardContext::getReaderNames(const unsigned char **mszReaders, size_t *readersLg) {
  *readersLg = readerNamesLg;
  *mszReaders = readerNames;
}

DWORD WinscardContext::attachReader(string new_reader) {
  unsigned int next = 0;

  auto new_reader_impl = SmartcardReader::instance_of(new_reader);

  if (new_reader_impl == nullptr)
    return SCARD_E_UNKNOWN_READER;

  for (auto reader : readers) {
    if (reader.second->getName() == new_reader_impl->getName()) {
      next++;
    }
  }
  new_reader_impl->setId(next);
  readers[new_reader_impl->getReaderIdentifier()] = new_reader_impl;

  // readerEvents.notify();

  refreshReaderNames();

  return SCARD_S_SUCCESS;
}

DWORD WinscardContext::insertSmartCardIn(const string &reader, const string &card) {
  try {
    DWORD ret = readers.at(reader)->insertCard(card);

    return ret;
  }
  catch (out_of_range &oor) {
    return SCARD_E_READER_UNAVAILABLE;
  }
}

DWORD WinscardContext::removeSmartCardFrom(const string &reader) {
  try {
    DWORD ret = readers.at(reader)->ejectCard();

    return ret;
  }
  catch (out_of_range &oor) {
    return SCARD_E_READER_UNAVAILABLE;
  }
}

DWORD WinscardContext::connectToSmartCard(const char *readerName, DWORD dwShareMode, DWORD dwPreferredProtocols, LPSCARDHANDLE phCard, LPDWORD pdwActiveProtocol) {
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

DWORD WinscardContext::disconnectFromSmartCard(SCARDHANDLE hCard, DWORD dwDisposition) {

  try {
    DWORD ret = smartcards_ctx.at(hCard)->reader->disconnectFromSmartCard(smartcards_ctx.at(hCard)->scardhandle, dwDisposition);
    smartcards_ctx.erase(hCard);
    return ret;
  }
  catch (out_of_range &oor) {
    return static_cast<DWORD>(SCARD_E_INVALID_HANDLE);
  }
}

DWORD WinscardContext::beginTransactionOnSmartcard(SCARDHANDLE hCard) {

  try {
    DWORD ret = smartcards_ctx.at(hCard)->reader->beginTransactionOnSmartcard(smartcards_ctx.at(hCard)->scardhandle);
    return ret;
  }
  catch (out_of_range &oor) {
    return static_cast<DWORD>(SCARD_E_INVALID_HANDLE);
  }
}

DWORD WinscardContext::endTransactionOnSmartcard(SCARDHANDLE hCard, DWORD dwDisposition) {

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

DWORD WinscardContext::smartcardStatus(SCARDHANDLE hCard, LPSTR mszReaderName, LPDWORD pcchReaderLen, LPDWORD pdwState, LPDWORD pdwProtocol, LPBYTE pbAtr, LPDWORD pcbAtrLen) {
  try {
    return smartcards_ctx.at(hCard)->reader->smartcardStatus(hCard, mszReaderName, pcchReaderLen, pdwState, pdwProtocol, pbAtr, pcbAtrLen);
  }
  catch (out_of_range &oor) {
    return static_cast<DWORD>(SCARD_E_INVALID_HANDLE);
  }
}

// TODO: No support for multithreaded SCardGetStatusChange! Need a vector of promises or condition variables
DWORD WinscardContext::contextGetStatusChange(DWORD dwTimeout, SCARD_READERSTATE *rgReaderStates, DWORD cReaders) {
  {
    std::mutex m;
    shared_ptr<WinscardEventObserver> observer = make_shared<WinscardEventObserver>(rgReaderStates, cReaders);
    // readerEvents.attach(observer);
    observer->wait_for(m, dwTimeout);
  }

  return SCARD_E_UNEXPECTED;
}

void WinscardContext::refreshReaderNames() {

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