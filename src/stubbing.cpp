//
// Created by david on 6/10/17.
//
#include <map>
#include <cstring>
#include <memory>
#include "stubbing.h"

using namespace std;

/**
 * The stubbing interface to be implemented for different ways of stubbing
 */
class Stubbing {
public:
  Stubbing() = default;

  Stubbing(Stubbing &other) = delete;

  Stubbing(Stubbing &&other) = delete;

  Stubbing &operator=(Stubbing &other) = delete;

  Stubbing &operator=(Stubbing &&other) = delete;

  virtual void set_return_code_for(const char *function, long ret) = 0;

  virtual long get_return_code_for(const char *function, long default_ret) = 0;

  virtual void clear_return_code_for(const char *function) = 0;

  virtual void clear_return_codes() = 0;

  virtual void set_out_parameter_for(const char *function, const char *parameter, const unsigned char *data, unsigned long data_lg) = 0;

  virtual long get_out_parameter_for(const char *function, const char *parameter, const unsigned char **data, unsigned long *data_lg) = 0;

  virtual void clear_out_parameter_for(const char *function, const char *parameter) = 0;

  virtual void clear_out_parameters() = 0;

  virtual void clear_all() = 0;

  virtual ~Stubbing() = 0;

  /**
   * Static member function for the factory method
   * @param impl
   * @return
   */
  static unique_ptr<Stubbing> instance_of(string &impl);
};

inline Stubbing::~Stubbing() = default;

/**
 * This class implements the stubbing interface using map implementation
 */
class StubbingMemory : public Stubbing {
public:
  class MemBuffer {
  public:
    MemBuffer(const unsigned char *data, size_t data_lg) {
      buffer = new unsigned char[data_lg];
      memcpy(buffer, data, data_lg);
      buffer_size = data_lg;
    }

    MemBuffer(const MemBuffer &data) : buffer_size(0), buffer(nullptr) {
      copy(data);
    }

    MemBuffer &operator=(const MemBuffer &data) {
      copy(data);
      return *this;
    }

    MemBuffer(MemBuffer &&data) noexcept : buffer_size(0), buffer(nullptr) {
      // Move values
      buffer_size = data.buffer_size;
      buffer = data.buffer;
      // Reset originals
      data.buffer_size = 0;
      data.buffer = nullptr;
    }

    MemBuffer &operator=(MemBuffer &&data) noexcept {
      if (this!=&data) {
        // free local
        delete[] buffer;
        buffer_size = 0;
        // Move values
        buffer_size = data.buffer_size;
        buffer = data.buffer;
        // Reset the originale
        data.buffer_size = 0;
        data.buffer = nullptr;
      }
      return *this;
    }

    ~MemBuffer() {
      delete[] buffer;
    }

    const unsigned char *getBuffer() const { return buffer; };

    const size_t getBufferLg() const { return buffer_size; };

  private:

    unsigned char *buffer;
    size_t buffer_size;

    void copy(const MemBuffer &data) {
      buffer = new unsigned char(data.getBufferLg());
      memcpy(buffer, data.getBuffer(), data.getBufferLg());
      buffer_size = data.getBufferLg();
    }

  };

  StubbingMemory() = default;

  StubbingMemory(StubbingMemory &other) = delete;

  StubbingMemory(StubbingMemory &&other) = delete;

  StubbingMemory &operator=(StubbingMemory &other) = delete;

  StubbingMemory &operator=(StubbingMemory &&other) = delete;

  void set_return_code_for(const char *function, long ret) override {
    return_codes[function] = ret;
  }

  long get_return_code_for(const char *function, long default_ret) override {
    map<string, long>::const_iterator return_codes_it = return_codes.find(function);
    if ( return_codes_it!=return_codes.end() ) {
      return return_codes_it->second;
    }

    return default_ret;
  }

  void clear_return_code_for(const char *function) override {
    return_codes.erase(function);
  }

  void clear_return_codes() override {
    return_codes.clear();
  }

  void set_out_parameter_for(const char *function, const char *parameter, const unsigned char *data, size_t data_lg) override {
    out_parameters[function][parameter] = new MemBuffer(data, data_lg);
  }

  long get_out_parameter_for(const char *function, const char *parameter, const unsigned char **data, unsigned long *data_lg) override {
    map<string, map<string, MemBuffer*>>::const_iterator out_parameters_it = out_parameters.find(function);
    if ( out_parameters_it!=out_parameters.end() ) {
      map<string, MemBuffer*> params = out_parameters_it->second;
      auto out_params_it = params.find(parameter);
      if (out_params_it!=params.end()) {
        *data = (*out_params_it).second->getBuffer();
        *data_lg = (*out_params_it).second->getBufferLg();
        return 1;
      }
    }
    *data = nullptr;
    *data_lg = 0;
    return 0;
  }

  void clear_out_parameter_for(const char *function, const char *parameter) override {
    auto function_it = out_parameters.find(function);

    if ( function_it!=out_parameters.end() ) {
      map<string, MemBuffer *>::const_iterator param_it = (function_it->second).find(parameter);
      if (param_it != (function_it->second).end()) {
        delete param_it->second;
        (function_it->second).erase(param_it);
      }
      out_parameters.erase(function_it);
    }
  }

  void clear_out_parameters() override {

    for (auto&& params : out_parameters) {
      for (auto&& param : params.second) {
        delete param.second;
      }
    }

    out_parameters.clear();
  }

  void clear_all() override {
    clear_return_codes();
    clear_out_parameters();
  }

  ~StubbingMemory() override {
    clear_all();
  }

private:
  map<std::string, long> return_codes;
  map<string, map<string, MemBuffer *>> out_parameters;
};


unique_ptr<Stubbing> Stubbing::instance_of(string &impl) {
  if (impl == "memory") {
    return unique_ptr<Stubbing>(new StubbingMemory());
  }
}

string stubbing_impl = string("memory");
map<string, unique_ptr<Stubbing>> g_modules;

#ifdef __cplusplus
extern "C" {
#endif

void set_return_code_for(const char *module, const char *function, long ret) {
  try {
    g_modules.at(module)->set_return_code_for(function, ret);
  }
  catch (out_of_range &e) {
    g_modules[module] = Stubbing::instance_of(stubbing_impl);
    g_modules.at(module)->set_return_code_for(function, ret);
  }
}

long get_return_code_for(const char *module, const char *function, long default_ret) {
  try {
    return g_modules.at(module)->get_return_code_for(function, default_ret);
  }
  catch (out_of_range &e) {
    return default_ret;
  }
}

void set_out_parameter_for(const char *module, const char *function, const char *parameter, const unsigned char *data, size_t data_lg) {
  try {
    g_modules.at(module)->set_out_parameter_for(function, parameter, data, data_lg);
  }
  catch (out_of_range &e) {
    g_modules[module] = Stubbing::instance_of(stubbing_impl);
    g_modules.at(module)->set_out_parameter_for(function, parameter, data, data_lg);
  }
}

long get_out_parameter_for(const char *module, const char *function, const char *parameter, const unsigned char **data, size_t *data_lg) {
  try {
    return g_modules.at(module)->get_out_parameter_for(function, parameter, data, data_lg);
  }
  catch (out_of_range &e) {
    return 0;
  }
}

void clear_return_codes(const char *module) {
  try {
    g_modules.at(module)->clear_return_codes();
  }
  catch (out_of_range &e) {
    // Ignore error
  }
}

void clear_out_parameters(const char *module) {
  try {
    g_modules.at(module)->clear_out_parameters();
  }
  catch (out_of_range &e) {
    // Ignore error
  }
}

void clear_stubbing(const char *module) {
  try {
    g_modules.at(module)->clear_all();
  }
  catch (out_of_range &e) {
    // Ignore erroe
  }
}

void clear_return_code_for(const char *module, const char *function) {
  try {
    g_modules.at(module)->clear_return_code_for(function);
  }
  catch (out_of_range &e) {
    // Ignore error
  }
}

void clear_out_parameter_for(const char *module, const char *function, const char *parameter) {
  try {
    g_modules.at(module)->clear_out_parameter_for(function, parameter);
  }
  catch (out_of_range &e) {
    // Ignore error
  }
}

#ifdef __cplusplus
};
#endif

SetReturnCodeFor::SetReturnCodeFor(const char *module, const char *function, long ret)
  : mod(module), func(function) {
  set_return_code_for(module, function, ret);
}

SetReturnCodeFor::~SetReturnCodeFor() {
  clear_return_code_for(mod.c_str(), func.c_str());
}

SetOutParameterFor::SetOutParameterFor(const char *module, const char *function, const char *parameter, const unsigned char *data, size_t data_lg)
  : mod(module), func(function), param(parameter) {
  set_out_parameter_for(module, function, parameter, data, data_lg);
}

SetOutParameterFor::~SetOutParameterFor() {
  clear_out_parameter_for(mod.c_str(), func.c_str(), param.c_str());
}