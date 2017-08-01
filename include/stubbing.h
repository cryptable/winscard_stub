//
// Created by david on 6/10/17.
//

#ifndef STUBBING_H
#define STUBBING_H
#include <string>

#ifdef __cplusplus
extern "C" {
#endif
void set_stubbing_active(const char *module);

void set_return_code_for(const char *module, const char *function, long ret);

long get_return_code_for(const char *module, const char *function, long default_ret);

void set_out_parameter_for(const char *module, const char *function, const char *parameter, const unsigned char *data, unsigned long data_lg);

long get_out_parameter_for(const char *module, const char *function, const char *parameter, const unsigned char **data, unsigned long *data_lg);

void clear_stubbing(const char *module);

#ifdef __cplusplus
};
#endif

/**
 * This class is a helper class to set a return code as long as the object instantiated from
 * SetReturnCodeFor exists.
 */
class SetReturnCodeFor {
public:

  /**
   * Constructor to set the return code for the function specified by the string <function>
   * @param function function which will return the return code
   * @param ret the return code to be returned
   */
  SetReturnCodeFor(const char *module, const char *function, long ret);

  /**
   * Destructor which will remove the stubbing of the return code
   */
  ~SetReturnCodeFor();

private:
  std::string mod;
  std::string func;
};

/**
 * This class is a helper class to set a return value as long as the object instantiated from
 * SetOutParameterFor exists.
 */
class SetOutParameterFor {

public:

  /**
   * Constructor which set the output parameter for the function specified by <function>. This is the basis for all
   * function like parameters, like strings, structs (not a deep copy!), etc...
   * @param module of which the function belongs
   * @param function function with the return parameter to be returned
   * @param param parameter which need to the specified data.
   * @param serialized data of the parameter to be returned
   * @param length of data of the parameter to be returned
   */
  SetOutParameterFor(const char *module, const char *function, const char *param, const unsigned char *data, size_t data_lg);

  /**
   * Destructor which will remove the stubbing of the return parameter
   */
  ~SetOutParameterFor();

private:
  std::string mod;
  std::string func;
  std::string param;
};
#endif //STUBBING_H
