
template <typename ErrorType, typename ... Args>
void throw_runtime_error(Args&& ... args) {
  static_assert(std::is_base_of_v<Exception, ErrorType>,
		"ErrorType must be derived from Exception");

  throw ErrorType(std::forward<Args>(args)...);
}


template <typename ErrorType, typename ... Args>
void throw_runtime_error_if(bool condition, Args&& ... args) {
  if (condition) throw_runtime_error<ErrorType>(std::forward<Args>(args)...);
}
  
template <typename ... Args>
Exception::Exception(Args&& ... args) {
  std::ostringstream oss;
  (oss << ... << args);
  _msg = oss.str();
}
    
inline std::string const &Exception::what() const {
  return _msg;
}

struct SystemLocked: Exception {
  SystemLocked(std::string const &obj1, std::string const &sig1,
	       std::string const &obj2, std::string const &sig2):
    Exception("Cannot connect signal \"", sig1, "\" of module \"", obj1,
	      "\" to  signal \"", sig2, "\" of module \"", obj2, "\" after call to System::init().")
  {}
};

struct OutputChangeNotAllowed: Exception {
  OutputChangeNotAllowed(std::string const &obj):
    Exception("Tried to change outputs of module \"", obj, "\" inside clock handler.")
  {}
};

struct IndexOutOfBounds: Exception {
  IndexOutOfBounds(std::string const &iotype, std::string const &obj, size_t index, size_t range):
    Exception("Signal index (", iotype, " ", index, ") of module \"", obj, "\" out of bounds. Number of ", iotype, "-signals of this module is ", range, ".")
  {}
};
  

struct InvalidSignalName: Exception {
  InvalidSignalName(std::string const &obj, std::string const &sig):
    Exception("Signal \"", sig, "\" is not available on module \"", obj, "\".")
  {}
    
};

struct DuplicateModuleNames: Exception {
  DuplicateModuleNames(std::string const &name):
    Exception("A module named \"", name, "\" was already part of the system.")
  {}
};
  
struct InvalidModuleName: Exception {
  InvalidModuleName(std::string const &name):
    Exception("Could not find a module by the name \"", name, "\".")
  {}
};

struct InvalidModuleType: Exception {
  InvalidModuleType(std::string const &name, std::string const &type):
    Exception("Module \"", name, "\" could not be cast to \"", type, "\".")
  {}
};

struct SystemNotInitialized: Exception {
  SystemNotInitialized():
    Exception("System was not initialized. Call System::init() before running the system.")
  {}
};

struct InvalidScopeName: Exception {
  InvalidScopeName(std::string const &name):
    Exception("Could not find a scope by the name \"", name, "\".")
  {}
};

struct DuplicateScopeNames: Exception {
  DuplicateScopeNames(std::string const &name):
    Exception("A scope named \"", name, "\" was already part of the system.")
  {}
};

struct SystemFrequencyOutOfRange: Exception {
  SystemFrequencyOutOfRange(double f, double fMin, double fMax):
    Exception("System frequency (", f, " Hz) must be in range in range ",
	      fMin, "Hz - ", (fMax / 1e6), " Mhz.")
  {}
};

