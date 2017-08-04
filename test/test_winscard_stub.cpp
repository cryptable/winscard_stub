//
// Created by david on 6/10/17.
//
#define CATCH_CONFIG_MAIN
#include <winscard.h>
#include "catch.hpp"
#include "stubbing.h"

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

TEST_CASE( "SCardListReaders() for default behaviour", "[API]") {
  SCARDCONTEXT hContext = 0;
  LONG         ret = 0;
  DWORD readersSize = 33;
  char defaultReaders[] = "Pinpad Reader\0Non Pinpad Reader\0\0";

  ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);

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

    REQUIRE( ret == SCARD_S_SUCCESS );
    REQUIRE( readersSize == tbcReadersSize );
  }

  SECTION("Check returned readers") {
    DWORD        tbcReadersSize = 33;
    char         tbcReaders[33] = { 0x00 };
    ret = SCardListReaders(hContext, NULL, tbcReaders, &tbcReadersSize);

    REQUIRE( ret == SCARD_S_SUCCESS );
    REQUIRE( readersSize == tbcReadersSize );
    REQUIRE( memcmp(tbcReaders, defaultReaders, 33) == 0 );
  }

  SECTION("Check invalid handle") {
    DWORD        tbcReadersSize = 33;
    char         tbcReaders[33] = { 0x00 };

    ret = SCardListReaders(hContext + 1, NULL, tbcReaders, &tbcReadersSize);

    REQUIRE( ret == SCARD_E_INVALID_HANDLE );
    REQUIRE( tbcReadersSize == 0 );
    REQUIRE( tbcReaders[0] == 0 );
  }

  ret = SCardReleaseContext(hContext);
}

TEST_CASE( "SCardConnect() testing for default behaviour", "[API]") {
  SCARDCONTEXT hContext = 0;
  LONG         ret = 0;
  char         defaultReader[] = "Non Pinpad Reader";

  ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);

  SECTION("Success") {
    SCARDHANDLE dwCardHandle = 0;
    DWORD       dwActiveProtocol = 0;

    ret = SCardConnect(hContext, defaultReader, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &dwCardHandle, &dwActiveProtocol);

    REQUIRE(ret == SCARD_S_SUCCESS);
    REQUIRE(dwCardHandle != 0);
    REQUIRE(dwActiveProtocol == SCARD_PROTOCOL_T0);
  }

  ret = SCardReleaseContext(hContext);
}