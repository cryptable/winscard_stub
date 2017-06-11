//
// Created by david on 6/10/17.
//
#include <map>
#include "stubbing.h"

using namespace std;

/**
 * The stubbing interface to be implemented for different ways of stubbing
 */
class Stubbing {
public:
  virtual void set_return_code_for(const char *function, long ret) = 0;

  virtual long get_return_code_for(const char *function, long default_ret) = 0;

  virtual void set_out_parameter_for(const char *function, const char *parameter, void* data) = 0;

  virtual void* get_out_parameter_for(const char *function, const char *parameter) = 0;

  virtual void clear_return_codes(void) = 0;

  virtual void clear_out_parameters(void) = 0;

  virtual void clear_all(void) = 0;
};

/**
 * This class implements the stubbing interface using map implementation
 */
class StubbingMemory : public Stubbing {
public:
  void set_return_code_for(const char *function, long ret) {
    return_codes[function] = ret;
  }

  long get_return_code_for(const char *function, long default_ret) {
    map<string, long>::const_iterator return_codes_it = return_codes.find(function);
    if ( return_codes_it!=return_codes.end() ) {
      return return_codes_it->second;
    }

    return default_ret;
  }

  void set_out_parameter_for(const char *function, const char *parameter, void* data) {
    out_parameters[function][parameter] = data;
  }

  void* get_out_parameter_for(const char *function, const char *parameter) {
    map<string, map<string, void*>>::const_iterator out_parameters_it = out_parameters.find(function);
    if ( out_parameters_it!=out_parameters.end() ) {
      map<string, void*> params = out_parameters_it->second;
      map<string, void*>::iterator out_params_it = params.find(parameter);
      if (out_params_it!=params.end()) {
        return out_params_it->second;
      }
    }

    return NULL;
  }

  void clear_return_codes(void) {
    return_codes.clear();
  }

  void clear_out_parameters(void) {
    out_parameters.clear();
  }

  void clear_all(void) {
    return_codes.clear();
    out_parameters.clear();
  }

private:
  map<std::string, long> return_codes;
  map<string, map<string, void*>> out_parameters;
};

#ifdef __cplusplus
extern "C" {
#endif
StubbingMemory stubbingMemory;
Stubbing &g_stub = stubbingMemory;

void set_return_code_for(const char *function, long ret) {
  g_stub.set_return_code_for(function, ret);
}

long get_return_code_for(const char *function, long default_ret) {
  return g_stub.get_return_code_for(function, default_ret);
}

void set_out_parameter_for(const char *function, const char *parameter, void* data) {
  g_stub.set_out_parameter_for(function, parameter, data);
}

void* get_out_parameter_for(const char *function, const char *parameter) {
  return g_stub.get_out_parameter_for(function, parameter);
}

void clear_return_codes(void) {
  g_stub.clear_return_codes();
}

void clear_out_parameters(void) {
  g_stub.clear_out_parameters();
}

void clear_stubbing(void) {
  g_stub.clear_all();
}

#ifdef __cplusplus
};
#endif

SetReturnCodeFor::SetReturnCodeFor(const char *function, long ret) {
  set_return_code_for(function, ret);
}

SetReturnCodeFor::~SetReturnCodeFor() {
  clear_return_codes();
}

SetOutParameterFor::SetOutParameterFor(const char *function, const char *param, void *data) {
  set_out_parameter_for(function, param, data);
}

SetOutParameterFor::~SetOutParameterFor() {
  clear_out_parameters();
}