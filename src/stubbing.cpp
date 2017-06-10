//
// Created by david on 6/10/17.
//
#include <map>
#include "stubbing.h"

using namespace std;

map<std::string, long> return_codes;
map<string, map<string, void*>> out_parameters;

#ifdef __cplusplus
extern "C" {
#endif

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

void clear_stubbing(void) {
  return_codes.clear();
  out_parameters.clear();
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

#ifdef __cplusplus
};
#endif

SetReturnCodeFor::SetReturnCodeFor(const char *function, long ret) {
  set_return_code_for(function, ret);
}

SetReturnCodeFor::~SetReturnCodeFor() {
  return_codes.clear();
}

SetOutParameterFor::SetOutParameterFor(const char *function, const char *param, void *data) {
  set_out_parameter_for(function, param, data);
}

SetOutParameterFor::~SetOutParameterFor() {
  out_parameters.clear();
}