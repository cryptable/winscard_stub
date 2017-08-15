//
// Created by david on 7/30/17.
//

#include <cstring>
#include "catch.hpp"
#include "stubbing.h"

TEST_CASE( "SetReturnCodeFor test", "[API]")  {

  clear_modules();

  SECTION("Success") {
    SetReturnCodeFor setReturnCodeFor("module", "functie", -1);

    REQUIRE( get_return_code_for("module", "functie", 0) == -1);
  }

  SECTION("Fail wrong module, so default value") {
    SetReturnCodeFor setReturnCodeFor("module", "functie", -1);

    REQUIRE( get_return_code_for("wrong_module", "functie", 0) == 0);
  }

  SECTION("Fail wrong function, so default value") {
    SetReturnCodeFor setReturnCodeFor("module", "functie", -1);

    REQUIRE( get_return_code_for("module", "wrong_functie", 0) == 0);
  }
}

TEST_CASE( "SetOutParameter test", "[API]") {

  clear_modules();

  SECTION("Success") {
    unsigned char ref_data[] = "Hello, world";
    size_t ref_data_lg = sizeof(ref_data);
    const unsigned char *data = nullptr;
    unsigned long data_lg = 0;

    SetOutParameterFor setOutParameterFor("module", "function", "param1", ref_data, ref_data_lg);

    long ret = get_out_parameter_for("module", "function", "param1", &data, &data_lg);

    REQUIRE( ret == 1 );
    REQUIRE( data_lg == ref_data_lg );
    REQUIRE( memcmp(data, ref_data, data_lg) == 0 );
  }

  SECTION("Success with NULL values") {
    unsigned char *ref_data = nullptr;
    size_t ref_data_lg = 0;
    const unsigned char *data = nullptr;
    unsigned long data_lg = 0;

    SetOutParameterFor setOutParameterFor("module", "function", "param1", ref_data, ref_data_lg);

    long ret = get_out_parameter_for("module", "function", "param1", &data, &data_lg);

    REQUIRE( ret == 1 );
    REQUIRE( data_lg == 0 );
    REQUIRE( data == nullptr );
  }

  SECTION("Fail wrong module, so default value") {
    unsigned char ref_data[] = "Hello, world";
    size_t ref_data_lg = sizeof(ref_data);
    const unsigned char *data = nullptr;
    unsigned long data_lg = 0;

    SetOutParameterFor setOutParameterFor("module", "function", "param1", ref_data, ref_data_lg);

    long ret = get_out_parameter_for("wrong_module", "function", "param1", &data, &data_lg);

    REQUIRE( ret == 0);
    REQUIRE( data_lg == 0 );
    REQUIRE( data == nullptr );
  }

  SECTION("Fail wrong function, so default value") {
    unsigned char ref_data[] = "Hello, world";
    size_t ref_data_lg = sizeof(ref_data);
    const unsigned char *data = nullptr;
    unsigned long data_lg = 0;

    SetOutParameterFor setOutParameterFor("module", "function", "param1", ref_data, ref_data_lg);

    long ret = get_out_parameter_for("module", "wrong_function", "param1", &data, &data_lg);

    REQUIRE( ret == 0);
    REQUIRE( data_lg == 0 );
    REQUIRE( data == nullptr );
  }

  SECTION("Fail wrong parameter, so default value") {
    unsigned char ref_data[] = "Hello, world";
    size_t ref_data_lg = sizeof(ref_data);
    const unsigned char *data = nullptr;
    unsigned long data_lg = 0;

    SetOutParameterFor setOutParameterFor("module", "function", "param1", ref_data, ref_data_lg);

    long ret = get_out_parameter_for("module", "function", "wrong_param", &data, &data_lg);

    REQUIRE( ret == 0);
    REQUIRE( data_lg == 0 );
    REQUIRE( data == nullptr );
  }
}

TEST_CASE( "Activate test", "[API]") {

  SECTION("Success default value") {

    REQUIRE( get_stubbing_active() == TRUE );
  }

  SECTION("Success settings false") {
    set_stubbing_active(FALSE);

    REQUIRE( get_stubbing_active() == FALSE );
  }

  SECTION("Success settings true") {
    set_stubbing_active(TRUE);

    REQUIRE( get_stubbing_active() == TRUE );
  }
}

TEST_CASE( "set_return_code_for test in C", "[API]") {

  clear_modules();

  SECTION("Success") {
    set_return_code_for("module", "functie", -1);

    REQUIRE( get_return_code_for("module", "functie", 0) == -1);
  }

  SECTION("Fail wrong module, so default value") {
    set_return_code_for("module", "functie", -1);

    REQUIRE( get_return_code_for("wrong_module", "functie", 0) == 0);
  }

  SECTION("Fail wrong function, so default value") {
    set_return_code_for("module", "functie", -1);

    REQUIRE( get_return_code_for("module", "wrong_functie", 0) == 0);
  }

  SECTION("Fail clear module, so default value") {
    set_return_code_for("module", "functie", -1);

    clear_stubbing("module");

    REQUIRE( get_return_code_for("module", "functie", 0) == 0);
  }

  SECTION("Fail clear all modules, so default value") {
    set_return_code_for("module", "functie", -1);

    clear_modules();

    REQUIRE( get_return_code_for("module", "functie", 0) == 0);
  }

  SECTION("Fail clear all return codes, so default value") {
    set_return_code_for("module", "functie", -1);

    clear_return_codes("module");

    REQUIRE( get_return_code_for("module", "functie", 0) == 0);
  }
}

TEST_CASE( "set_out_parameter_for test in C", "[API]") {

  clear_modules();

  SECTION("Success") {
    unsigned char ref_data[] = "Hello, world";
    size_t ref_data_lg = sizeof(ref_data);
    const unsigned char *data = nullptr;
    unsigned long data_lg = 0;

    set_out_parameter_for("module", "function", "param1", ref_data, ref_data_lg);

    long ret = get_out_parameter_for("module", "function", "param1", &data, &data_lg);

    REQUIRE( ret == 1 );
    REQUIRE( data_lg == ref_data_lg );
    REQUIRE( memcmp(data, ref_data, data_lg) == 0 );
  }

  SECTION("Success with NULL values") {
    unsigned char *ref_data = nullptr;
    size_t ref_data_lg = 0;
    const unsigned char *data = nullptr;
    unsigned long data_lg = 0;

    set_out_parameter_for("module", "function", "param1", ref_data, ref_data_lg);

    long ret = get_out_parameter_for("module", "function", "param1", &data, &data_lg);

    REQUIRE( ret == 1 );
    REQUIRE( data_lg == 0 );
    REQUIRE( data == nullptr );
  }

  SECTION("Fail wrong module, all null") {
    unsigned char ref_data[] = "Hello, world";
    size_t ref_data_lg = sizeof(ref_data);
    const unsigned char *data = nullptr;
    unsigned long data_lg = 0;

    set_out_parameter_for("module", "function", "param1", ref_data, ref_data_lg);

    long ret = get_out_parameter_for("wrong_module", "function", "param1", &data, &data_lg);

    REQUIRE( ret == 0);
    REQUIRE( data_lg == 0 );
    REQUIRE( data == nullptr );
  }

  SECTION("Fail wrong function, all null") {
    unsigned char ref_data[] = "Hello, world";
    size_t ref_data_lg = sizeof(ref_data);
    const unsigned char *data = nullptr;
    unsigned long data_lg = 0;

    set_out_parameter_for("module", "function", "param1", ref_data, ref_data_lg);

    long ret = get_out_parameter_for("module", "wrong_function", "param1", &data, &data_lg);

    REQUIRE( ret == 0);
    REQUIRE( data_lg == 0 );
    REQUIRE( data == nullptr );
  }

  SECTION("Fail wrong parameter, all null") {
    unsigned char ref_data[] = "Hello, world";
    size_t ref_data_lg = sizeof(ref_data);
    const unsigned char *data = nullptr;
    unsigned long data_lg = 0;

    set_out_parameter_for("module", "function", "param1", ref_data, ref_data_lg);

    long ret = get_out_parameter_for("module", "function", "wrong_param", &data, &data_lg);

    REQUIRE( ret == 0);
    REQUIRE( data_lg == 0 );
    REQUIRE( data == nullptr );
  }

  SECTION("Fail wrong clear module, all null") {
    unsigned char ref_data[] = "Hello, world";
    size_t ref_data_lg = sizeof(ref_data);
    const unsigned char *data = nullptr;
    unsigned long data_lg = 0;

    set_out_parameter_for("module", "function", "param1", ref_data, ref_data_lg);

    clear_stubbing("module");

    long ret = get_out_parameter_for("module", "function", "param1", &data, &data_lg);

    REQUIRE( ret == 0);
    REQUIRE( data_lg == 0 );
    REQUIRE( data == nullptr );
  }

  SECTION("Fail wrong all clear module, all null") {
    unsigned char ref_data[] = "Hello, world";
    size_t ref_data_lg = sizeof(ref_data);
    const unsigned char *data = nullptr;
    unsigned long data_lg = 0;

    set_out_parameter_for("module", "function", "param1", ref_data, ref_data_lg);

    clear_modules();

    long ret = get_out_parameter_for("module", "function", "param1", &data, &data_lg);

    REQUIRE( ret == 0);
    REQUIRE( data_lg == 0 );
    REQUIRE( data == nullptr );
  }

  SECTION("Fail clear all out parameters, all null") {
    unsigned char ref_data[] = "Hello, world";
    size_t ref_data_lg = sizeof(ref_data);
    const unsigned char *data = nullptr;
    unsigned long data_lg = 0;

    set_out_parameter_for("module", "function", "param1", ref_data, ref_data_lg);

    clear_out_parameters("module");

    long ret = get_out_parameter_for("module", "function", "param1", &data, &data_lg);

    REQUIRE( ret == 0);
    REQUIRE( data_lg == 0 );
    REQUIRE( data == nullptr );
  }
}