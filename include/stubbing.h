//
// Created by david on 6/10/17.
//

#ifndef STUBBING_H
#define STUBBING_H
#include <map>

#ifdef __cplusplus
extern "C" {
#endif

void set_return_code_for(const char *function, long ret);

long get_return_code_for(const char *function, long default_ret);

void set_out_parameter_for(const char *function, const char *parameter, void* data);

void* get_out_parameter_for(const char *function, const char *parameter);

void clear_stubbing(void);

#ifdef __cplusplus
};
#endif

class SetReturnCodeFor {
public:

  SetReturnCodeFor(const char *function, long ret);

  ~SetReturnCodeFor();
};

class SetOutParameterFor {
public:
  SetOutParameterFor(const char *function, const char *param, void *data);
  ~SetOutParameterFor();
};
#endif //STUBBING_H_H
