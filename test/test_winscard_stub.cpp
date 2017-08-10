//
// Created by david on 6/10/17.
//
#define CATCH_CONFIG_MAIN
#include <winscard.h>
#include <thread>
#include "catch.hpp"
#include "stubbing.h"
#include "winscard_stub.h"

TEST_CASE( "SCardEstablishContext() stubbing call", "[API]") {
  SCARDCONTEXT hContext = 0;
  LONG         ret = 0;

  SECTION("Success") {
    ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);

    REQUIRE( ret == SCARD_S_SUCCESS);

    ret = SCardReleaseContext(hContext);
  }

  SECTION("Failed return code") {
    SetReturnCodeFor setReturnCodeFor("winscard", "SCardEstablishContext", SCARD_E_INVALID_PARAMETER);

    ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);

    REQUIRE( ret == SCARD_E_INVALID_PARAMETER);

    ret = SCardReleaseContext(hContext);
  }

  SECTION("Default behaviour for invalid handle parameter") {

    ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, NULL);

    REQUIRE( ret == SCARD_E_INVALID_PARAMETER);
  }

  SECTION("Default behaviour for invalid scope parameter") {

    ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM + 10, NULL, NULL, &hContext);

    REQUIRE( ret == SCARD_E_INVALID_VALUE);

    ret = SCardReleaseContext(hContext);
  }
}

TEST_CASE( "SCardReleaseContext() stubbing call", "[API]") {
  SCARDCONTEXT hContext = 0;
  LONG         ret = 0;

  SECTION("Success") {
    // Arrange
    ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);

    // Act
    ret = SCardReleaseContext(hContext);

    // Assert
    REQUIRE(ret == SCARD_S_SUCCESS);
  }

  SECTION("Failed return code") {
    // Arrange
    ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
    SetReturnCodeFor setReturnCodeFor("winscard", "SCardReleaseContext", SCARD_E_INVALID_HANDLE);

    // Act
    ret = SCardReleaseContext(hContext);

    // Assert
    REQUIRE( ret == SCARD_E_INVALID_HANDLE);
  }

  SECTION("Failed default return code for invalid handle") {
    // Arrange
    ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);

    // Act
    ret = SCardReleaseContext(hContext+1);

    // Assert
    REQUIRE( ret == SCARD_E_INVALID_HANDLE);

    ret = SCardReleaseContext(hContext);
  }
}

TEST_CASE( "SCardIsValidContext() testing", "[API]") {
  SCARDCONTEXT hContext = 0;
  LONG         ret = 0;

  ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);

  SECTION("Success") {
    ret = SCardIsValidContext(hContext);

    REQUIRE(ret == SCARD_S_SUCCESS);
  }

  SECTION("Failed return code using stubbing"){
    SetReturnCodeFor setReturnCodeFor("winscard", "SCardIsValidContext", SCARD_E_INVALID_HANDLE);

    ret = SCardIsValidContext(hContext);

    REQUIRE(ret == SCARD_E_INVALID_HANDLE);
  }

  SECTION("Failed return code using invalid handle"){
    SetReturnCodeFor setReturnCodeFor("winscard", "SCardIsValidContext", SCARD_E_INVALID_HANDLE);

    ret = SCardIsValidContext(hContext + 100);

    REQUIRE(ret == SCARD_E_INVALID_HANDLE);
  }

  SECTION("Failed return code using invalid handle"){
    // Arrange
    SCARDCONTEXT hContext2 = 0;
    SetReturnCodeFor setReturnCodeFor("winscard", "SCardIsValidContext", SCARD_E_INVALID_HANDLE);
    ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext2);
    ret = SCardReleaseContext(hContext2);

    // Act
    ret = SCardIsValidContext(hContext2);

    // Assert
    REQUIRE(ret == SCARD_E_INVALID_HANDLE);
  }

  ret = SCardReleaseContext(hContext);
}

TEST_CASE( "SCardListReaders() for readers", "[API]") {
  SCARDCONTEXT hContext = 0;
  LONG         ret = 0;
  const unsigned char readers[] = "Reader 1\0";
  DWORD readersSize = 10;

  ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);

  SetOutParameterFor setOutParameterFor1("winscard", "SCardListReaders", "mszReaders", readers, readersSize);

  SECTION("Success") {
    ret = SCardListReaders(hContext, NULL, NULL, NULL);

    REQUIRE(ret == SCARD_S_SUCCESS);
  }

  SECTION("Failed return code"){
    SetReturnCodeFor setReturnCodeFor("winscard", "SCardListReaders", SCARD_E_INVALID_HANDLE);

    ret = SCardListReaders(hContext, NULL, NULL, NULL);

    REQUIRE(ret == SCARD_E_INVALID_HANDLE);
  }

  SECTION("Check returned size") {
    DWORD        tbcReadersSize = 0;
    ret = SCardListReaders(hContext, NULL, NULL, &tbcReadersSize);

    REQUIRE(readersSize == tbcReadersSize);
  }

  SECTION("Check returned readers") {
    DWORD        tbcReadersSize = 10;
    char         tbcReaders[32] = { 0x00 };
    ret = SCardListReaders(hContext, NULL, tbcReaders, &tbcReadersSize);

    REQUIRE(readersSize == tbcReadersSize);
    REQUIRE(strcmp(tbcReaders, "Reader 1") == 0);
  }

  ret = SCardReleaseContext(hContext);
}

bool verifyReaders(LPCSTR mszReaders1, LPCSTR mszReaders2) {
  const char *p1 = mszReaders1;
  const char *p2 = mszReaders2;
  bool found = false;
  while (*p1 != '\0') {
    while (*p2 != '\0') {
      if (strncmp(p1, p2, strlen(p1)) == 0) {
        found = true;
        break;
      }
      p2 += strlen(p2) + 1;
    }
    if (!found)
      return false;
    found = false;
    p2 = mszReaders2;
    p1 += strlen(p1) + 1;
  }
  return true;
}

TEST_CASE( "SCardListReaders() for default behaviour", "[API]") {
  SCARDCONTEXT hContext = 0;
  LONG         ret = 0;
  DWORD readersSize = 37;
  char defaultReaders[] = "Non Pinpad Reader 0\0Pinpad Reader 0\0\0";

  ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);

  SECTION("Success") {
    ret = SCardAttachReader(hContext, "Pinpad Reader");
    ret = SCardAttachReader(hContext, "Non Pinpad Reader");

    ret = SCardListReaders(hContext, NULL, NULL, NULL);

    REQUIRE(ret == SCARD_S_SUCCESS);
  }

  SECTION("Failed return code"){
    ret = SCardAttachReader(hContext, "Pinpad Reader");
    ret = SCardAttachReader(hContext, "Non Pinpad Reader");

    SetReturnCodeFor setReturnCodeFor("winscard", "SCardListReaders", SCARD_E_INVALID_HANDLE);

    ret = SCardListReaders(hContext, NULL, NULL, NULL);

    REQUIRE(ret == SCARD_E_INVALID_HANDLE);
  }

  SECTION("Check returned size") {
    DWORD        tbcReadersSize = 0;
    ret = SCardAttachReader(hContext, "Pinpad Reader");
    ret = SCardAttachReader(hContext, "Non Pinpad Reader");

    ret = SCardListReaders(hContext, NULL, NULL, &tbcReadersSize);

    REQUIRE( ret == SCARD_S_SUCCESS );
    REQUIRE( readersSize == tbcReadersSize );
  }

  SECTION("Check returned readers") {
    DWORD        tbcReadersSize = readersSize;
    char         tbcReaders[sizeof(defaultReaders)] = { 0x00 };
    ret = SCardAttachReader(hContext, "Pinpad Reader");
    ret = SCardAttachReader(hContext, "Non Pinpad Reader");

    ret = SCardListReaders(hContext, NULL, tbcReaders, &tbcReadersSize);

    REQUIRE( ret == SCARD_S_SUCCESS );
    REQUIRE( readersSize == tbcReadersSize );
    REQUIRE( verifyReaders(defaultReaders, tbcReaders) );
  }

  SECTION("Check invalid handle") {
    DWORD        tbcReadersSize = readersSize;
    char         tbcReaders[sizeof(defaultReaders)] = { 0x00 };

    ret = SCardAttachReader(hContext, "Pinpad Reader");
    ret = SCardAttachReader(hContext, "Non Pinpad Reader");
    ret = SCardListReaders(hContext + 1, NULL, tbcReaders, &tbcReadersSize);

    REQUIRE( ret == SCARD_E_INVALID_HANDLE );
    REQUIRE( tbcReadersSize == 0 );
    REQUIRE( tbcReaders[0] == 0 );
  }

  SECTION("Check no readers attached") {
    DWORD        tbcReadersSize = readersSize;
    char         tbcReaders[sizeof(defaultReaders)] = { 0x00 };

    ret = SCardListReaders(hContext, NULL, tbcReaders, &tbcReadersSize);

    REQUIRE( ret == SCARD_E_NO_READERS_AVAILABLE );
    REQUIRE( tbcReadersSize == 0 );
    REQUIRE( tbcReaders[0] == 0 );
  }

  ret = SCardReleaseContext(hContext);
}

TEST_CASE( "SCardConnect() testing for default behaviour", "[API]") {
  SCARDCONTEXT hContext = 0;
  LONG         ret = 0;
  char         defaultReader[] = "Non Pinpad Reader 0";

  ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
  ret = SCardAttachReader(hContext, "Non Pinpad Reader");
  ret = SCardAttachReader(hContext, "Pinpad Reader");
  ret = SCardInsertSmartCardInReader(hContext, "Non Pinpad Reader 0", "test");

  SECTION("Success") {
    SCARDHANDLE dwCardHandle = 0;
    DWORD       dwActiveProtocol = 0;

    ret = SCardConnect(hContext, defaultReader, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &dwCardHandle, &dwActiveProtocol);

    REQUIRE(ret == SCARD_S_SUCCESS);
    REQUIRE(dwCardHandle != 0);
    REQUIRE(dwActiveProtocol == SCARD_PROTOCOL_T0);

    ret= SCardDisconnect(dwCardHandle, SCARD_LEAVE_CARD);
  }

  SECTION("Success with second reader") {
    SCARDHANDLE dwCardHandle = 0;
    DWORD       dwActiveProtocol = 0;

    ret = SCardAttachReader(hContext, "Pinpad Reader");
    ret = SCardInsertSmartCardInReader(hContext, "Pinpad Reader 1", "test");

    ret = SCardConnect(hContext, "Pinpad Reader 1", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &dwCardHandle, &dwActiveProtocol);

    REQUIRE(ret == SCARD_S_SUCCESS);
    REQUIRE(dwCardHandle != 0);
    REQUIRE(dwActiveProtocol == SCARD_PROTOCOL_T0);

    ret= SCardDisconnect(dwCardHandle, SCARD_LEAVE_CARD);
  }

  SECTION("Failed with invalid reader") {
    SCARDHANDLE dwCardHandle = 0;
    DWORD       dwActiveProtocol = 0;

    ret = SCardConnect(hContext, "Wrong Reader", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &dwCardHandle, &dwActiveProtocol);

    REQUIRE(ret == SCARD_E_UNKNOWN_READER);
  }

  SECTION("Failed with empty reader") {
    SCARDHANDLE dwCardHandle = 0;
    DWORD       dwActiveProtocol = 0;

    ret = SCardConnect(hContext, "Pinpad Reader 0", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &dwCardHandle, &dwActiveProtocol);

    REQUIRE(ret == SCARD_E_NO_SMARTCARD);
  }

  SECTION("Failed with invalid handle") {
    SCARDHANDLE dwCardHandle = 0;
    DWORD       dwActiveProtocol = 0;

    ret = SCardConnect(hContext + 1, "Pinpad Reader 0", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &dwCardHandle, &dwActiveProtocol);

    REQUIRE(ret == SCARD_E_INVALID_HANDLE);
  }

  SECTION("Failed with invalid shared mode") {
    SCARDHANDLE dwCardHandle = 0;
    DWORD       dwActiveProtocol = 0;

    ret = SCardInsertSmartCardInReader(hContext, "Pinpad Reader 0", "test");

    ret = SCardConnect(hContext, "Pinpad Reader 0", SCARD_SHARE_EXCLUSIVE, SCARD_PROTOCOL_T0, &dwCardHandle, &dwActiveProtocol);

    REQUIRE(ret == SCARD_E_INVALID_VALUE);
  }

  SECTION("Failed with invalid protocol") {
    SCARDHANDLE dwCardHandle = 0;
    DWORD       dwActiveProtocol = 0;

    ret = SCardInsertSmartCardInReader(hContext, "Pinpad Reader 0", "test");

    ret = SCardConnect(hContext, "Pinpad Reader 0", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T1, &dwCardHandle, &dwActiveProtocol);

    REQUIRE(ret == SCARD_E_INVALID_VALUE);
  }

  ret = SCardReleaseContext(hContext);
}

TEST_CASE( "SCardAttachReader() testing for default behaviour", "[API]") {
  SCARDCONTEXT hContext = 0;
  LONG         ret = 0;

  ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);

  SECTION("Success") {
    char defaultReaders[] = "Non Pinpad Reader 0\0";
    char tbcReaders[sizeof(defaultReaders)] = { 0x00 };
    DWORD tbcReadersLg = sizeof(defaultReaders);

    ret = SCardAttachReader(hContext, "Non Pinpad Reader");
    REQUIRE( ret == SCARD_S_SUCCESS );

    ret = SCardListReaders(hContext, NULL, tbcReaders, &tbcReadersLg);

    REQUIRE( sizeof(defaultReaders) == tbcReadersLg );
    REQUIRE( verifyReaders(tbcReaders, defaultReaders) );
  }

  SECTION("Success with 2 different readers") {
    char defaultReaders[] = "Non Pinpad Reader 0\0Pinpad Reader 0\0";
    char tbcReaders[sizeof(defaultReaders)] = { 0x00 };
    DWORD tbcReadersLg = sizeof(defaultReaders);

    ret = SCardAttachReader(hContext, "Non Pinpad Reader");
    REQUIRE( ret == SCARD_S_SUCCESS );
    ret = SCardAttachReader(hContext, "Pinpad Reader");
    REQUIRE( ret == SCARD_S_SUCCESS );

    ret = SCardListReaders(hContext, NULL, tbcReaders, &tbcReadersLg);

    REQUIRE( sizeof(defaultReaders) == tbcReadersLg );
    REQUIRE( verifyReaders(tbcReaders, defaultReaders) );
  }

  SECTION("Success with 3 readers where 2 the same") {
    char defaultReaders[] = "Non Pinpad Reader 0\0Pinpad Reader 0\0Pinpad Reader 1\0";
    char tbcReaders[sizeof(defaultReaders)] = { 0x00 };
    DWORD tbcReadersLg = sizeof(defaultReaders);

    ret = SCardAttachReader(hContext, "Non Pinpad Reader");
    REQUIRE( ret == SCARD_S_SUCCESS );
    ret = SCardAttachReader(hContext, "Pinpad Reader");
    REQUIRE( ret == SCARD_S_SUCCESS );
    ret = SCardAttachReader(hContext, "Pinpad Reader");
    REQUIRE( ret == SCARD_S_SUCCESS );

    ret = SCardListReaders(hContext, NULL, tbcReaders, &tbcReadersLg);

    REQUIRE( sizeof(defaultReaders) == tbcReadersLg );
    REQUIRE( verifyReaders(tbcReaders, defaultReaders) );
  }

  SECTION("Fail with unknown reader") {
    char defaultReaders[] = "Non Pinpad Reader 0\0";
    char tbcReaders[sizeof(defaultReaders)] = { 0x00 };
    DWORD tbcReadersLg = sizeof(defaultReaders);

    ret = SCardAttachReader(hContext, "Unknown Pinpad Reader");
    REQUIRE( ret == SCARD_E_UNKNOWN_READER );
  }

  SECTION("Fail with invalid handle") {
    char defaultReaders[] = "Non Pinpad Reader 0\0";
    char tbcReaders[sizeof(defaultReaders)] = { 0x00 };
    DWORD tbcReadersLg = sizeof(defaultReaders);

    ret = SCardAttachReader(hContext + 1, "Unknown Pinpad Reader");
    REQUIRE( ret == SCARD_E_INVALID_HANDLE );
  }

  ret = SCardReleaseContext(hContext);
}

TEST_CASE( "SCardInsertSmartCardInReader() testing for default behaviour", "[API]") {
  SCARDCONTEXT hContext = 0;
  LONG         ret = 0;

  ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);

  SECTION("Success") {
    ret = SCardAttachReader(hContext, "Non Pinpad Reader");

    ret = SCardInsertSmartCardInReader(hContext, "Non Pinpad Reader 0", "test");
    REQUIRE( ret == SCARD_S_SUCCESS );
  }

  SECTION("Success with 2 different readers") {
    ret = SCardAttachReader(hContext, "Non Pinpad Reader");
    ret = SCardAttachReader(hContext, "Pinpad Reader");

    ret = SCardInsertSmartCardInReader(hContext, "Non Pinpad Reader 0", "test");
    REQUIRE( ret == SCARD_S_SUCCESS );
  }

  SECTION("Success with 3 readers where 2 same readers") {
    ret = SCardAttachReader(hContext, "Non Pinpad Reader");
    ret = SCardAttachReader(hContext, "Pinpad Reader");
    ret = SCardAttachReader(hContext, "Pinpad Reader");

    ret = SCardInsertSmartCardInReader(hContext, "Pinpad Reader 1", "test");
    REQUIRE( ret == SCARD_S_SUCCESS );
  }

  SECTION("Fail with unknown card") {
    ret = SCardAttachReader(hContext, "Non Pinpad Reader");

    ret = SCardInsertSmartCardInReader(hContext, "Non Pinpad Reader 0", "unknown");
    REQUIRE( ret == SCARD_E_CARD_UNSUPPORTED );
  }

  SECTION("Fail with unknown reader") {
    ret = SCardAttachReader(hContext, "Non Pinpad Reader");

    ret = SCardInsertSmartCardInReader(hContext, "Non Pinpad Reader 1", "test");
    REQUIRE( ret == SCARD_E_READER_UNAVAILABLE );
  }

  SECTION("Fail with invalid handle") {
    ret = SCardAttachReader(hContext, "Non Pinpad Reader");

    ret = SCardInsertSmartCardInReader(hContext + 1, "Non Pinpad Reader 0", "test");
    REQUIRE( ret == SCARD_E_INVALID_HANDLE );
  }

  SECTION("Fail with card already in reader") {
    ret = SCardAttachReader(hContext, "Non Pinpad Reader");

    ret = SCardInsertSmartCardInReader(hContext, "Non Pinpad Reader 0", "test");
    REQUIRE( ret == SCARD_S_SUCCESS );
    ret = SCardInsertSmartCardInReader(hContext, "Non Pinpad Reader 0", "test");
    REQUIRE( ret == SCARD_E_CARD_IN_READER );
  }
  ret = SCardReleaseContext(hContext);
}

TEST_CASE( "SCardRemoveSmartCardFromReader() testing for default behaviour", "[API]") {
  SCARDCONTEXT hContext = 0;
  LONG         ret = 0;

  ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);

  SECTION("Success") {
    ret = SCardAttachReader(hContext, "Non Pinpad Reader");
    ret = SCardInsertSmartCardInReader(hContext, "Non Pinpad Reader 0", "test");

    ret = SCardRemoveSmartCardFromReader(hContext, "Non Pinpad Reader 0");
    REQUIRE( ret == SCARD_S_SUCCESS );
  }

  SECTION("Fail with unknown reader") {
    ret = SCardAttachReader(hContext, "Non Pinpad Reader");

    ret = SCardInsertSmartCardInReader(hContext, "Non Pinpad Reader 0", "test");

    ret = SCardRemoveSmartCardFromReader(hContext, "Non Pinpad Reader 1");
    REQUIRE( ret == SCARD_E_READER_UNAVAILABLE );
  }

  SECTION("Fail with unknown handle") {
    ret = SCardAttachReader(hContext, "Non Pinpad Reader");

    ret = SCardInsertSmartCardInReader(hContext, "Non Pinpad Reader 0", "test");

    ret = SCardRemoveSmartCardFromReader(hContext + 1, "Non Pinpad Reader 0");
    REQUIRE( ret == SCARD_E_INVALID_HANDLE );
  }

  SECTION("Fail card already removed") {
    ret = SCardAttachReader(hContext, "Non Pinpad Reader");

    ret = SCardInsertSmartCardInReader(hContext, "Non Pinpad Reader 0", "test");

    ret = SCardRemoveSmartCardFromReader(hContext, "Non Pinpad Reader 0");

    ret = SCardRemoveSmartCardFromReader(hContext, "Non Pinpad Reader 0");
    REQUIRE( ret == SCARD_E_NO_SMARTCARD );
  }

  ret = SCardReleaseContext(hContext);
}

TEST_CASE( "SCardDisconnect() testing for default behaviour", "[API]") {
  SCARDCONTEXT hContext = 0;
  LONG         ret = 0;

  ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
  ret = SCardAttachReader(hContext, "Non Pinpad Reader");
  ret = SCardAttachReader(hContext, "Pinpad Reader");
  ret = SCardInsertSmartCardInReader(hContext, "Non Pinpad Reader 0", "test");
  ret = SCardInsertSmartCardInReader(hContext, "Pinpad Reader 0", "test");

  SECTION("Success") {
    SCARDHANDLE dwCardHandle = 0;
    DWORD       dwActiveProtocol = 0;
    DWORD       dwDisposition = SCARD_LEAVE_CARD;
    ret = SCardConnect(hContext, "Non Pinpad Reader 0", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &dwCardHandle, &dwActiveProtocol);

    ret = SCardDisconnect(dwCardHandle, dwDisposition);
    REQUIRE(ret == SCARD_S_SUCCESS);
  }

  SECTION("Success mutliple cardhandles") {
    SCARDHANDLE dwCardHandle1 {0};
    SCARDHANDLE dwCardHandle2 {0};
    DWORD       dwActiveProtocol {0};
    DWORD       dwDisposition = SCARD_LEAVE_CARD;
    ret = SCardConnect(hContext, "Non Pinpad Reader 0", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &dwCardHandle1, &dwActiveProtocol);
    REQUIRE(ret == SCARD_S_SUCCESS);
    ret = SCardConnect(hContext, "Pinpad Reader 0", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &dwCardHandle2, &dwActiveProtocol);
    REQUIRE(ret == SCARD_S_SUCCESS);

    ret = SCardDisconnect(dwCardHandle2, dwDisposition);
    REQUIRE(ret == SCARD_S_SUCCESS);
    ret = SCardDisconnect(dwCardHandle1, dwDisposition);
    REQUIRE(ret == SCARD_S_SUCCESS);
  }

  SECTION("Failed mutliple cardhandles disconnect 2 times the same card") {
    SCARDHANDLE dwCardHandle1 {0};
    SCARDHANDLE dwCardHandle2 {0};
    DWORD       dwActiveProtocol {0};
    DWORD       dwDisposition = SCARD_LEAVE_CARD;
    ret = SCardConnect(hContext, "Non Pinpad Reader 0", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &dwCardHandle1, &dwActiveProtocol);
    REQUIRE(ret == SCARD_S_SUCCESS);
    ret = SCardConnect(hContext, "Pinpad Reader 0", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &dwCardHandle2, &dwActiveProtocol);
    REQUIRE(ret == SCARD_S_SUCCESS);

    ret = SCardDisconnect(dwCardHandle2, dwDisposition);
    REQUIRE(ret == SCARD_S_SUCCESS);
    ret = SCardDisconnect(dwCardHandle2, dwDisposition);
    REQUIRE(ret == SCARD_E_INVALID_HANDLE);
    ret = SCardDisconnect(dwCardHandle1, dwDisposition);
    REQUIRE(ret == SCARD_S_SUCCESS);
  }

  SECTION("Success disconnect with ejecting the card") {
    SCARDHANDLE dwCardHandle {0};
    DWORD       dwActiveProtocol {0};
    DWORD       dwDisposition = SCARD_EJECT_CARD;

    ret = SCardConnect(hContext, "Non Pinpad Reader 0", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &dwCardHandle, &dwActiveProtocol);
    REQUIRE(ret == SCARD_S_SUCCESS);
    ret = SCardDisconnect(dwCardHandle, dwDisposition);
    REQUIRE(ret == SCARD_S_SUCCESS);
    ret = SCardConnect(hContext, "Non Pinpad Reader 0", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &dwCardHandle, &dwActiveProtocol);
    REQUIRE(ret == SCARD_E_NO_SMARTCARD);
  }

  SECTION("Fail with invalid handle") {
    SCARDHANDLE dwCardHandle {0};
    DWORD       dwActiveProtocol {0};
    DWORD       dwDisposition = SCARD_EJECT_CARD;

    ret = SCardConnect(hContext, "Non Pinpad Reader 0", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &dwCardHandle, &dwActiveProtocol);
    REQUIRE(ret == SCARD_S_SUCCESS);
    ret = SCardDisconnect(dwCardHandle + 1, dwDisposition);
    REQUIRE(ret == SCARD_E_INVALID_HANDLE);
    ret = SCardDisconnect(dwCardHandle, dwDisposition);
  }

  ret = SCardReleaseContext(hContext);
}

TEST_CASE( "SCardBeginTransaction() testing for default behaviour", "[API]") {
  SCARDCONTEXT hContext = 0;
  LONG         ret = 0;
  SCARDHANDLE dwCardHandle = 0;
  DWORD       dwActiveProtocol = 0;
  DWORD       dwDisposition = SCARD_LEAVE_CARD;

  ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
  ret = SCardAttachReader(hContext, "Non Pinpad Reader");
  ret = SCardAttachReader(hContext, "Pinpad Reader");
  ret = SCardInsertSmartCardInReader(hContext, "Non Pinpad Reader 0", "test");
  ret = SCardConnect(hContext, "Non Pinpad Reader 0", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &dwCardHandle, &dwActiveProtocol);

  SECTION("Success") {
    ret = SCardBeginTransaction(dwCardHandle);

    REQUIRE(ret == SCARD_S_SUCCESS);

    ret = SCardEndTransaction(dwCardHandle, SCARD_LEAVE_CARD);
  }

  SECTION("Fail invalid handle") {
    ret = SCardBeginTransaction(dwCardHandle + 1);

    REQUIRE(ret == SCARD_E_INVALID_HANDLE);
  }

  SECTION("Fail 2 calls to SCardBeginTransaction") {
    ret = SCardBeginTransaction(dwCardHandle);

    ret = SCardBeginTransaction(dwCardHandle);
    REQUIRE(ret == SCARD_E_SHARING_VIOLATION);
  }

  SECTION("Fail with sharing violation, transaction already begun") {
    SCARDHANDLE dwCardHandle2 = 0;

    ret = SCardInsertSmartCardInReader(hContext, "Pinpad Reader 0", "test");
    ret = SCardConnect(hContext, "Pinpad Reader 0", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &dwCardHandle2, &dwActiveProtocol);

    ret = SCardBeginTransaction(dwCardHandle2);
    REQUIRE(ret == SCARD_S_SUCCESS);
    ret = SCardEndTransaction(dwCardHandle2, SCARD_EJECT_CARD);
    REQUIRE(ret == SCARD_S_SUCCESS);
    ret = SCardBeginTransaction(dwCardHandle2);
    REQUIRE(ret == SCARD_E_NO_SMARTCARD);

  }

  ret = SCardDisconnect(dwCardHandle, dwDisposition);
  ret = SCardReleaseContext(hContext);
}

TEST_CASE( "SCardEndTransaction() testing for default behaviour", "[API]") {
  SCARDCONTEXT hContext = 0;
  LONG         ret = 0;
  SCARDHANDLE dwCardHandle = 0;
  DWORD       dwActiveProtocol = 0;
  DWORD       dwDisposition = SCARD_LEAVE_CARD;

  ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
  ret = SCardAttachReader(hContext, "Non Pinpad Reader");
  ret = SCardAttachReader(hContext, "Pinpad Reader");
  ret = SCardInsertSmartCardInReader(hContext, "Non Pinpad Reader 0", "test");
  ret = SCardConnect(hContext, "Non Pinpad Reader 0", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &dwCardHandle, &dwActiveProtocol);

  SECTION("Success") {
    ret = SCardBeginTransaction(dwCardHandle);
    ret = SCardEndTransaction(dwCardHandle, SCARD_LEAVE_CARD);
    REQUIRE(ret == SCARD_S_SUCCESS);
  }

  SECTION("Fail invalid handle") {
    ret = SCardBeginTransaction(dwCardHandle);

    ret = SCardEndTransaction(dwCardHandle + 1, SCARD_LEAVE_CARD);

    REQUIRE(ret == SCARD_E_INVALID_HANDLE);
    ret = SCardEndTransaction(dwCardHandle, SCARD_LEAVE_CARD);
  }

  SECTION("Fail with missing SCardBeginTransaction") {

    ret = SCardEndTransaction(dwCardHandle, SCARD_LEAVE_CARD);

    REQUIRE(ret == SCARD_E_NOT_TRANSACTED);
  }

  SECTION("Fail with 2 times calling SCardEndTransaction") {
    ret = SCardBeginTransaction(dwCardHandle);
    ret = SCardEndTransaction(dwCardHandle, SCARD_LEAVE_CARD);

    ret = SCardEndTransaction(dwCardHandle, SCARD_LEAVE_CARD);
    REQUIRE(ret == SCARD_E_NOT_TRANSACTED);
  }

  SECTION("Fail with SCARD_W_RESET_CARD") {
    ret = SCardBeginTransaction(dwCardHandle);

    ret = SCardEndTransaction(dwCardHandle, SCARD_RESET_CARD);
    REQUIRE(ret == SCARD_W_RESET_CARD);
  }

  SECTION("Fail with SCARD_W_RESET_CARD") {
    ret = SCardBeginTransaction(dwCardHandle);
    ret = SCardRemoveSmartCardFromReader(hContext, "Non Pinpad Reader 0");

    ret = SCardEndTransaction(dwCardHandle, SCARD_RESET_CARD);
    REQUIRE(ret == SCARD_E_NO_SMARTCARD);
  }

  ret = SCardDisconnect(dwCardHandle, dwDisposition);
  ret = SCardReleaseContext(hContext);
}

TEST_CASE( "SCardStatus() testing for default behaviour", "[API]") {
  SCARDCONTEXT hContext { 0 };
  LONG         ret { 0 };

  ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
  ret = SCardAttachReader(hContext, "Non Pinpad Reader");

  SECTION("Success") {
    SCARDHANDLE hCard { 0 };
    DWORD       dwActiveProtocol { 0 };
    char        *reader = nullptr;
    DWORD       readerLg = sizeof(reader);
    DWORD       state { 0 };
    DWORD       protocol { 0 };
    BYTE        *atr = nullptr;
    BYTE        atr_ref[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16 };
    DWORD       atrLg { 0 };

    ret = SCardInsertSmartCardInReader(hContext, "Non Pinpad Reader 0", "test");
    ret = SCardConnect(hContext, "Non Pinpad Reader 0", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &hCard, &dwActiveProtocol);

    ret = SCardStatus(hCard, NULL, &readerLg, &state, &protocol, NULL, &atrLg);
    REQUIRE( ret == SCARD_E_INSUFFICIENT_BUFFER);
    REQUIRE( protocol == SCARD_PROTOCOL_T0 );
    REQUIRE( (0x00FF & state) == SCARD_SPECIFIC );
    REQUIRE( readerLg == 18);
    REQUIRE( atrLg == 16);

    reader = new char[readerLg];
    atr = new BYTE[atrLg];

    ret = SCardStatus(hCard, reader, &readerLg, &state, &protocol, atr, &atrLg);
    REQUIRE( ret == SCARD_S_SUCCESS);
    REQUIRE( protocol == SCARD_PROTOCOL_T0 );
    REQUIRE( (0x00FF & state) == SCARD_SPECIFIC );
    REQUIRE( strcmp(reader, "Non Pinpad Reader") == 0 );
    REQUIRE( atrLg == 16);
    REQUIRE( memcmp(atr, atr_ref, atrLg) == 0);

    delete reader;
    delete atr;
    ret = SCardDisconnect(hCard, SCARD_EJECT_CARD);
  }

  SECTION("Success with pre-allocated reader") {
    SCARDHANDLE hCard { 0 };
    DWORD       dwActiveProtocol { 0 };
    char        reader[33] { 0x00 };
    DWORD       readerLg = sizeof(reader);
    DWORD       state { 0 };
    DWORD       protocol { 0 };
    BYTE        *atr;
    BYTE        atr_ref[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16 };
    DWORD       atrLg { 0 };

    ret = SCardInsertSmartCardInReader(hContext, "Non Pinpad Reader 0", "test");
    ret = SCardConnect(hContext, "Non Pinpad Reader 0", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &hCard, &dwActiveProtocol);

    ret = SCardStatus(hCard, reader, &readerLg, &state, &protocol, NULL, &atrLg);
    REQUIRE( ret == SCARD_E_INSUFFICIENT_BUFFER);
    REQUIRE( protocol == SCARD_PROTOCOL_T0 );
    REQUIRE( (0x00FF & state) == SCARD_SPECIFIC );
    REQUIRE( strcmp(reader, "Non Pinpad Reader") == 0 );
    REQUIRE( atrLg == 16);

    atr = new BYTE[atrLg];
    ret = SCardStatus(hCard, reader, &readerLg, &state, &protocol, atr, &atrLg);
    REQUIRE( ret == SCARD_S_SUCCESS);
    REQUIRE( protocol == SCARD_PROTOCOL_T0 );
    REQUIRE( (0x00FF & state) == SCARD_SPECIFIC );
    REQUIRE( strcmp(reader, "Non Pinpad Reader") == 0 );
    REQUIRE( atrLg == 16);
    REQUIRE( memcmp(atr, atr_ref, atrLg) == 0);

    delete atr;
    ret = SCardDisconnect(hCard, SCARD_EJECT_CARD);
  }

  SECTION("Success with pre-allocated atr buffer") {
    SCARDHANDLE hCard { 0 };
    DWORD       dwActiveProtocol { 0 };
    char        *reader = nullptr;
    DWORD       readerLg = 0;
    DWORD       state { 0 };
    DWORD       protocol { 0 };
    BYTE        atr[MAX_ATR_SIZE] { 0x00 };
    BYTE        atr_ref[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16 };
    DWORD       atrLg { MAX_ATR_SIZE };

    ret = SCardInsertSmartCardInReader(hContext, "Non Pinpad Reader 0", "test");
    ret = SCardConnect(hContext, "Non Pinpad Reader 0", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &hCard, &dwActiveProtocol);

    ret = SCardStatus(hCard, NULL, &readerLg, &state, &protocol, atr, &atrLg);
    REQUIRE( ret == SCARD_E_INSUFFICIENT_BUFFER);
    REQUIRE( protocol == SCARD_PROTOCOL_T0 );
    REQUIRE( (0x00FF & state) == SCARD_SPECIFIC );
    REQUIRE( readerLg == 18);
    REQUIRE( atrLg == 16);

    reader = new char[readerLg];

    ret = SCardStatus(hCard, reader, &readerLg, &state, &protocol, atr, &atrLg);
    REQUIRE( ret == SCARD_S_SUCCESS);
    REQUIRE( protocol == SCARD_PROTOCOL_T0 );
    REQUIRE( (0x00FF & state) == SCARD_SPECIFIC );
    REQUIRE( strcmp(reader, "Non Pinpad Reader") == 0 );
    REQUIRE( memcmp(atr, atr_ref, atrLg) == 0);

    delete reader;
    ret = SCardDisconnect(hCard, SCARD_EJECT_CARD);
  }

  SECTION("Success with card remove/inserted") {
    SCARDHANDLE hCard { 0 };
    DWORD       dwActiveProtocol { 0 };
    char        reader[20] { 0x00 };
    DWORD       readerLg = sizeof(reader);
    DWORD       state { 0 };
    DWORD       protocol { 0 };
    BYTE        atr[MAX_ATR_SIZE] = { 0x00 };
    DWORD       atrLg { MAX_ATR_SIZE };
    DWORD       eventCntr1 = 0;
    DWORD       eventCntr2 = 0;

    ret = SCardInsertSmartCardInReader(hContext, "Non Pinpad Reader 0", "test");
    ret = SCardConnect(hContext, "Non Pinpad Reader 0", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &hCard, &dwActiveProtocol);

    ret = SCardStatus(hCard, reader, &readerLg, &state, &protocol, atr, &atrLg);
    eventCntr1 = 0x0000FFFF & (state >> 16);
    REQUIRE( ret == SCARD_S_SUCCESS);
    REQUIRE( eventCntr1 > 0 );
    ret = SCardRemoveSmartCardFromReader(hContext, "Non Pinpad Reader 0");
    ret = SCardInsertSmartCardInReader(hContext, "Non Pinpad Reader 0", "test");

    ret = SCardStatus(hCard, reader, &readerLg, &state, &protocol, atr, &atrLg);
    eventCntr2= 0x0000FFFF & (state >> 16);
    REQUIRE( ret == SCARD_S_SUCCESS);
    REQUIRE( eventCntr2 > eventCntr1 );

    ret = SCardDisconnect(hCard, SCARD_EJECT_CARD);
  }

  SECTION("Fail with card absent") {
    SCARDHANDLE hCard { 0 };
    DWORD       dwActiveProtocol { 0 };
    DWORD       readerLg = 0;
    DWORD       state { 0 };
    DWORD       protocol { 0 };
    DWORD       atrLg { MAX_ATR_SIZE };

    ret = SCardInsertSmartCardInReader(hContext, "Non Pinpad Reader 0", "test");
    ret = SCardConnect(hContext, "Non Pinpad Reader 0", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &hCard, &dwActiveProtocol);
    ret = SCardRemoveSmartCardFromReader(hContext, "Non Pinpad Reader 0");

    ret = SCardStatus(hCard, NULL, &readerLg, &state, &protocol, NULL, &atrLg);
    REQUIRE( ret == SCARD_W_REMOVED_CARD);
    REQUIRE( (0x00FF & state) == SCARD_ABSENT );

    ret = SCardDisconnect(hCard, SCARD_EJECT_CARD);
  }

  SECTION("Fail with invalid handle") {
    SCARDHANDLE hCard { 0 };
    DWORD       dwActiveProtocol { 0 };
    DWORD       readerLg = 0;
    DWORD       state { 0 };
    DWORD       protocol { 0 };
    DWORD       atrLg { MAX_ATR_SIZE };

    ret = SCardInsertSmartCardInReader(hContext, "Non Pinpad Reader 0", "test");
    ret = SCardConnect(hContext, "Non Pinpad Reader 0", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &hCard, &dwActiveProtocol);

    ret = SCardStatus(hCard + 1, NULL, &readerLg, &state, &protocol, NULL, &atrLg);
    REQUIRE( ret == SCARD_E_INVALID_HANDLE);

    ret = SCardDisconnect(hCard, SCARD_EJECT_CARD);
  }

  ret = SCardReleaseContext(hContext);
}

TEST_CASE( "SCardGetStatusChange() testing for default behaviour", "[API]") {
  SCARDCONTEXT hContext{0};
  LONG ret{0};

  ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);

  SECTION("Succes attach reader", "[API]") {
    SCARD_READERSTATE readerStates[1] {};
    DWORD             readerStatesLg {sizeof(readerStates)};

    std::thread th1([hContext]{
      DWORD ret = 0;
      std::this_thread::sleep_for(std::chrono::seconds(1));
      ret = SCardAttachReader(hContext, "Non Pinpad Reader");
      REQUIRE(ret == SCARD_S_SUCCESS);
    });

    readerStates[0].szReader = "\\\\?PnP?\\Notification";
    ret = SCardGetStatusChange(hContext, 10, readerStates, readerStatesLg);
    REQUIRE( ret == SCARD_S_SUCCESS );
    REQUIRE( readerStates[0].dwEventState == SCARD_STATE_CHANGED );
    REQUIRE( strcmp(readerStates[0].szReader,"Non Pinpad Reader 0") == 0 );

    th1.join();
  }

  ret = SCardReleaseContext(hContext);
}