//
// Created by david on 6/10/17.
//

#ifndef STUBBING_H
#define STUBBING_H
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

/**
 * Set the stubbing active or not. Depends on the implementation if it uses the activation
 * @param active (FALSE:0 to deactivate / TRUE:1 to activate), default it is activated
 */
void set_stubbing_active(int active);

/**
 * Used by the stubbing implementation to check the stubbing activation
 * @return (FALSE:0 to deactivate / TRUE:1 to activate), default it is activated
 */
int get_stubbing_active();

/**
 * Set the return code for the function in the module (library) of the SDK which is stubbed
 * @param module name of the stubbed module (library)
 * @param function name of the stubbed function in the module
 * @param ret return code which must be return by the function
 */
void set_return_code_for(const char *module, const char *function, long ret);

/**
 * Get the return code for usage in the stubbed function in the module
 * @param module name of the stubbed module (library)
 * @param function name of the stubbed function in the module
 * @param default_ret default return code when return code is not set
 * @return
 */
long get_return_code_for(const char *module, const char *function, long default_ret);

void set_out_parameter_for(const char *module, const char *function, const char *parameter, const unsigned char *data, unsigned long data_lg);

long get_out_parameter_for(const char *module, const char *function, const char *parameter, const unsigned char **data, unsigned long *data_lg);

void clear_return_codes(const char *module);

void clear_out_parameters(const char *module);

void clear_stubbing(const char *module);

void clear_modules();

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
