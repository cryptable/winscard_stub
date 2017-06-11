//
// Created by david on 6/10/17.
//
#define CATCH_CONFIG_MAIN
#include <winscard.h>
#include "catch.hpp"
#include "stubbing.h"

TEST_CASE( "SCardEstablishContext() success stubbing call", "[API]") {
  SCARDCONTEXT hContext = 0;
  LONG         ret = 0;

  ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);

  SECTION("Success") {
    ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);

    REQUIRE( ret == SCARD_S_SUCCESS);
  }

  SECTION("Failed return code") {
    SetReturnCodeFor setReturnCodeFor("SCardEstablishContext", SCARD_E_INVALID_PARAMETER);

    ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);

    REQUIRE( ret == SCARD_E_INVALID_PARAMETER);
  }
}

TEST_CASE( "SCardReleaseContext() failed stubbing call", "[API]") {
  SCARDCONTEXT hContext = 101;
  LONG         ret = 0;

  ret = SCardReleaseContext(hContext);

  SECTION("Success") {
    ret = SCardReleaseContext(hContext);

    REQUIRE(ret == SCARD_S_SUCCESS);
  }

  SECTION("Failed return code") {
    SetReturnCodeFor setReturnCodeFor("SCardReleaseContext", SCARD_E_INVALID_HANDLE);

    ret = SCardReleaseContext(hContext);

    REQUIRE( ret == SCARD_E_INVALID_HANDLE);
  }
}

TEST_CASE( "SCardListReaders() for one reader only", "[API]") {
  SCARDCONTEXT hContext = 101;
  LONG         ret = 0;
  const char   readers[] = "Reader 1\0";
  DWORD        readersSize = 10;

  SetOutParameterFor setOutParameterFor1("SCardListReaders", "mszReaders", (void *)readers);
  SetOutParameterFor setOutParameterFor2("SCardListReaders", "pcchReaders", (void *)readersSize);

  SECTION("Success") {
    ret = SCardListReaders(hContext, NULL, NULL, NULL);

    REQUIRE(ret == SCARD_S_SUCCESS);
  }

  SECTION("Failed return code"){
    SetReturnCodeFor setReturnCodeFor("SCardListReaders", SCARD_E_INVALID_HANDLE);

    ret = SCardListReaders(hContext, NULL, NULL, NULL);

    REQUIRE(ret == SCARD_E_INVALID_HANDLE);
  }

  SECTION("Check returned size") {
    DWORD        tbcReadersSize = 0;
    ret = SCardListReaders(hContext, NULL, NULL, &tbcReadersSize);

    REQUIRE(readersSize == tbcReadersSize);
  }

  SECTION("Check returned readers") {
    DWORD        tbcReadersSize = 0;
    char         tbcReaders[32] = { 0x00 };
    ret = SCardListReaders(hContext, NULL, tbcReaders, &tbcReadersSize);

    REQUIRE(readersSize == tbcReadersSize);
    REQUIRE(strcmp(tbcReaders, "Reader 1") == 0);
  }
}