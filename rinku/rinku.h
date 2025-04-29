#ifndef RINKU_H
#define RINKU_H

#include <vector>
#include <cassert>
#include <cstdint>
#include <algorithm>
#include <numeric>
#include <memory>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <format>
#include <chrono>
#include <unordered_set>
#include <unordered_map>
#include <cmath>

#define UNREACHABLE__ assert(false && "UNREACHABLE");

#define RINKU_INPUT(SIGNAL, N)				\
  struct SIGNAL: Rinku::Impl::Input_<N, SIGNAL> {	\
    static constexpr char const *Name = #SIGNAL;	\
  };

#define RINKU_OUTPUT(SIGNAL, N)				\
  struct SIGNAL: Rinku::Impl::Output_<N, SIGNAL> {	\
    static constexpr char const *Name = #SIGNAL;	\
  };

#define RINKU_SIGNAL_LIST(NAME, ...) using NAME = Rinku::Impl::Signals_< __VA_ARGS__ >;
#define RINKU_MODULE(...) public Rinku::Module< __VA_ARGS__ >
#define RINKU_ON_CLOCK_RISING() virtual void clockRising() override
#define RINKU_ON_CLOCK_FALLING() virtual void clockFalling() override
#define RINKU_UPDATE() virtual void update([[maybe_unused]] GuaranteeToken guarantee_no_get_input) override
#define RINKU_GUARANTEE_NO_GET_INPUT() guarantee_no_get_input.set();
#define RINKU_RESET() virtual void reset() override
#define RINKU_NOT(SIGNAL) Rinku::Not<SIGNAL>

#define RINKU_PP_GET_3RD(_1, _2, _3, NAME, ...) NAME
#define RINKU_PP_GET_2ND(_1, _2, NAME, ...) NAME

#define RINKU_SYSTEM_HALT2(HLT_SIGNAL, MODULE)	connectHalt<HLT_SIGNAL>(MODULE)
#define RINKU_SYSTEM_HALT3(SYSTEM, HLT_SIGNAL, MODULE)(SYSTEM).connectHalt<HLT_SIGNAL>(MODULE)
#define RINKU_SYSTEM_HALT(...)			\
  RINKU_PP_GET_3RD(__VA_ARGS__,			\
		   RINKU_SYSTEM_HALT3,		\
		   RINKU_SYSTEM_HALT2)		\
  (__VA_ARGS__)

#define RINKU_SYSTEM_ERROR2(ERR_SIGNAL, MODULE)	connectError<ERR_SIGNAL>(MODULE)
#define RINKU_SYSTEM_ERROR3(SYSTEM, ERR_SIGNAL, MODULE)	(SYSTEM).connectError<ERR_SIGNAL>(MODULE)
#define RINKU_SYSTEM_ERROR(...)			\
  RINKU_PP_GET_3RD(__VA_ARGS__,			\
		   RINKU_SYSTEM_ERROR3,		\
		   RINKU_SYSTEM_ERROR2)		\
  (__VA_ARGS__)

#define RINKU_SYSTEM_EXIT2(EXIT_SIGNAL, MODULE)	connectExit<EXIT_SIGNAL>(MODULE)
#define RINKU_SYSTEM_EXIT3(SYSTEM, EXIT_SIGNAL, MODULE)	(SYSTEM).connectExit<EXIT_SIGNAL>(MODULE)
#define RINKU_SYSTEM_EXIT(...)	       \
  RINKU_PP_GET_3RD(__VA_ARGS__,	       \
		   RINKU_SYSTEM_EXIT3, \
		   RINKU_SYSTEM_EXIT2) \
  (__VA_ARGS__)

#define RINKU_SYSTEM_EXIT_CODE2(EXIT_CODE_SIGNAL, MODULE) connectExitCode<EXIT_CODE_SIGNAL>(MODULE)
#define RINKU_SYSTEM_EXIT_CODE3(SYSTEM, EXIT_CODE_SIGNAL, MODULE) (SYSTEM).connectExitCode<EXIT_CODE_SIGNAL>(MODULE)
#define RINKU_SYSTEM_EXIT_CODE(...)		\
  RINKU_PP_GET_3RD(__VA_ARGS__,			\
		   RINKU_SYSTEM_EXIT_CODE3,	\
		   RINKU_SYSTEM_EXIT_CODE2)	\
  (__VA_ARGS__)


#define RINKU_ADD_MODULE1(MODULE_TYPE) addModule<MODULE_TYPE>
#define RINKU_ADD_MODULE2(SYSTEM, MODULE_TYPE) (SYSTEM).addModule<MODULE_TYPE>
#define RINKU_ADD_MODULE(...)	      \
  RINKU_PP_GET_2ND(__VA_ARGS__,	      \
		   RINKU_ADD_MODULE2, \
		   RINKU_ADD_MODULE1) \
    (__VA_ARGS__)

#define RINKU_CONNECT_MOD(INPUT_OBJECT, INPUT_SIGNAL, OUTPUT_OBJECT, OUTPUT_SIGNAL) \
  (INPUT_OBJECT).connect<INPUT_SIGNAL, OUTPUT_SIGNAL>((OUTPUT_OBJECT))

#define RINKU_CONNECT_CONST(INPUT_OBJECT, INPUT_SIGNAL, VALUE)	\
  (INPUT_OBJECT).connect<INPUT_SIGNAL, VALUE>()

#define RINKU_GET_OUTPUT(OUTPUT_SIGNAL) getOutput<OUTPUT_SIGNAL>()
#define RINKU_GET_OUTPUT_INDEX(INDEX) getOutput((INDEX), -1)			
#define RINKU_SET_OUTPUT(OUTPUT_SIGNAL, VALUE) setOutput<OUTPUT_SIGNAL>((VALUE))
#define RINKU_SET_OUTPUT_INDEX(INDEX, VALUE) setOutput((INDEX), (VALUE), -1)
#define RINKU_GET_INPUT(INPUT_SIGNAL) getInput<INPUT_SIGNAL>()
#define RINKU_GET_INPUT_INDEX(INDEX) getInput((INDEX), -1)


#ifdef RINKU_REMOVE_MACRO_PREFIX
#define INPUT RINKU_INPUT
#define OUTPUT RINKU_OUTPUT
#define SIGNAL_LIST RINKU_SIGNAL_LIST
#define MODULE RINKU_MODULE
#define ON_CLOCK_RISING RINKU_ON_CLOCK_RISING 
#define ON_CLOCK_FALLING RINKU_ON_CLOCK_FALLING
#define UPDATE RINKU_UPDATE
#define RESET RINKU_RESET
#define NOT RINKU_NOT
#define ADD_MODULE RINKU_ADD_MODULE
#define SYSTEM_HALT RINKU_SYSTEM_HALT
#define SYSTEM_ERROR RINKU_SYSTEM_ERROR
#define SYSTEM_EXIT RINKU_SYSTEM_EXIT
#define CONNECT_MOD RINKU_CONNECT_MOD
#define CONNECT_CONST RINKU_CONNECT_CONST
#define GET_OUTPUT RINKU_GET_OUTPUT
#define GET_OUTPUT_INDEX RINKU_GET_OUTPUT_INDEX
#define SET_OUTPUT RINKU_SET_OUTPUT
#define SET_OUTPUT_INDEX RINKU_SET_OUTPUT_INDEX
#define GET_INPUT RINKU_GET_INPUT
#define GET_INPUT_INDEX RINKU_GET_INPUT_INDEX
#define GUARANTEE_NO_GET_INPUT RINKU_GUARANTEE_NO_GET_INPUT
#endif

namespace Rinku {

  using signal_t = uint64_t;

  namespace Error {
    
    class Exception {
      std::string _msg;

    public:
      template <typename ... Args>
      Exception(Args&& ... args) {
	std::ostringstream oss;
	(oss << ... << args);
	_msg = oss.str();
      }
    
      std::string const &what() const {
	return _msg;
      }
    };

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
  } // namespace Error
  
  namespace Impl {
    template <size_t N, typename Base_, bool isInput>
    struct IO_ {
      static constexpr bool IsInput = isInput;
      static constexpr bool IsOutput = !isInput;
      static constexpr size_t Width = N;
      static constexpr signal_t Mask = (N == 64 ? -1 : (1 << N) - 1);
      using Base = Base_;
    };
    
    template <size_t N, typename Base_>
    struct Input_: IO_<N, Base_, true>
    {};

    template <size_t N, typename Base_, bool Inverted = false>
    struct Output_: IO_<N, Base_, false>
    {
      static constexpr bool ActiveLow = Inverted;
    };

    template <typename ... Args>
    class Signals_ {
    public:
      static constexpr size_t N = sizeof ... (Args);
  
      template <typename S, size_t I = 0>
      static consteval size_t index_of() {
	if constexpr (I == N) {
	  static_assert(I != N, "Signal type not found in this Signals_ list.");
	  return -1; // unreachable
	} else if constexpr (std::is_same_v<S, std::tuple_element_t<I, std::tuple<Args...>>>) {
	  return I;
	} else {
	  return index_of<S, I + 1>();
	}
      }

      static consteval bool is_input_list() {
	return (Args::IsInput && ...);
      }

      static consteval bool is_output_list() {
	return (Args::IsOutput && ...);
      }
      
      static_assert(is_input_list() || is_output_list(),
		    "Signal list must contain only inputs or outputs.");

      static char const * const *names() {
	static constexpr char const *_names[] = {
	  (Args::Name)...
	};
	return _names;
      }

      static signal_t const *masks() {
	static constexpr signal_t const _masks[] = {
	  (Args::Mask)...
	};
	return _masks;
      };

      static size_t const *widths() {
	static constexpr size_t const _widths[] = {
	  (Args::Width)...
	};
	return _widths;
      };

    }; // class Signal_

    class ModuleBase {
      bool _locked = false;
      bool _setOutputAllowed = true;
      bool _guaranteed = false;
      int _index = -1;
      std::string _name = "[UNNAMED]";
      
    public:
      class GuaranteeToken {
	friend class ModuleBase;
	bool *value;
	GuaranteeToken(bool *val):
	  value(val)
	{}
      public:
	void set() {
	  assert(value && "value can't be null");
	  *value = true;
	}
      };
      
      virtual ~ModuleBase() = default;
      virtual void clockRising() {}
      virtual void clockFalling() {}
      virtual void update(GuaranteeToken) {}
      virtual void reset() {}
      virtual std::vector<int> updateAndCheck() = 0;

      void update() {
	this->update(GuaranteeToken{&_guaranteed});
      }

      void setModuleIndex(int idx) {
	_index = idx;
      }

      int getModuleIndex() const {
	return _index;
      }

      void setName(std::string const &name) {
	_name = name;
      }

      std::string const &name() const {
	return _name;
      }
      
      void lock() {
	_locked = true;
      }

      bool locked() const {
	return _locked;
      }

      void allowSetOutput(bool value) {
	_setOutputAllowed = value;
      }

      bool setOutputAllowed() const {
	return _setOutputAllowed;
      }

      void resetGuaranteed() {
	_guaranteed = false;
      }

      bool guaranteed() const {
	return _guaranteed;
      }
    }; // class ModuleBase


  } // namespace Impl


  template <typename Output>
  struct Not: Impl::Output_<Output::Width, typename Output::Base, !Output::ActiveLow>
  {
    static_assert(Output::IsOutput, "Cannot negate input signals.");
    static constexpr char const *Name = "NOT";
  };
  
  template <typename SignalList1 = Impl::Signals_<>,
	    typename SignalList2 = Impl::Signals_<>>
  class Module: public Impl::ModuleBase {

    template <typename, typename>
    friend class Module;

    friend class System;
    
  public:
    using Inputs = std::conditional_t<
      SignalList1::is_input_list() || SignalList2::is_input_list(),
      std::conditional_t<SignalList1::is_input_list(), SignalList1, SignalList2>,
      Impl::Signals_<>
    >;
  
    using Outputs = std::conditional_t<
      SignalList1::is_output_list() || SignalList2::is_output_list(),
      std::conditional_t<SignalList1::is_output_list(), SignalList1, SignalList2>,
      Impl::Signals_<>
    >;

    template <typename S, typename SignalList = std::conditional_t<S::IsInput, Inputs, Outputs>>
    static constexpr size_t index_of = SignalList::template index_of<S>();

  private:
    signal_t outputState[Outputs::N] {};
    std::vector<int> outputModules[Outputs::N] {};
    std::vector<std::pair<signal_t const*, bool>> inputState[Inputs::N];
    std::unordered_map<std::string, std::pair<size_t, signal_t>> nameToInput;
    std::unordered_map<std::string, std::pair<size_t, signal_t>> nameToOutput;
    
    void addOutgoing(size_t outputIndex, int idx) {
      for (int moduleIndex: outputModules[outputIndex])
	if (moduleIndex == idx) return;

      outputModules[outputIndex].push_back(idx);
    }

    bool connected(size_t inputIndex, signal_t const *ptr) {
      for (auto const &pr: inputState[inputIndex])
	if (pr.first == ptr) 
	  return true;
      return false;
    }

    std::vector<int> const &outgoing(size_t outputIndex) {
      return outputModules[outputIndex];
    }

    virtual std::vector<int> updateAndCheck() override final {
      if (guaranteed()) return {};
      
      signal_t oldOutputs[Outputs::N];
      for (size_t idx = 0; idx != Outputs::N; ++idx) {
	oldOutputs[idx] = outputState[idx];
      }

      this->update();

      std::vector<int> affected;
      for (size_t idx = 0; idx != Outputs::N; ++idx) {
	if (oldOutputs[idx] != outputState[idx]) {
	  auto const &outVec = outgoing(idx);
	  affected.insert(affected.end(), outVec.begin(), outVec.end());
	}
      }

      return affected;
    }

  public:
    using ModuleBase::name;
    
    Module() {
      for (size_t idx = 0; idx != Inputs::N; ++idx) {
	nameToInput.emplace(std::string(Inputs::names()[idx]),
			    std::pair<size_t, signal_t>(idx, Inputs::masks()[idx]));
      }

      for (size_t idx = 0; idx != Outputs::N; ++idx) {
	nameToOutput.emplace(std::string(Outputs::names()[idx]),
			     std::pair<size_t, signal_t>(idx, Outputs::masks()[idx]));
      }
    }
    
    template <typename InputSignal, typename OutputSignal, typename OtherModule>
    void connect(OtherModule &other) {
      static_assert(std::is_base_of_v<Impl::ModuleBase, OtherModule>,
		    "Other module is not a valid Rinku::Module.");
      assert(getModuleIndex() != -1 && "input module index not set");
      assert(other.getModuleIndex() != -1 && "output module index not set");


      Error::throw_runtime_error_if
	<Error::SystemLocked>(locked(), name(), InputSignal::Name, other.name(), OutputSignal::Name);
			    
      
      constexpr size_t inputIndex = index_of<InputSignal>;
      constexpr size_t outputIndex = OtherModule::template index_of<typename OutputSignal::Base>;

      signal_t *ptr = &other.outputState[outputIndex];
      if (connected(inputIndex, ptr)) return;
    
      other.addOutgoing(outputIndex, getModuleIndex());
      inputState[inputIndex].push_back({ptr, OutputSignal::ActiveLow});
    }

    template <typename InputSignal, signal_t Value>
    void connect() {
      static signal_t const value = Value;

      static_assert(Value < (1 << InputSignal::Width),
		    "Constant input value too large for signal-width.");

      Error::throw_runtime_error_if
	<Error::SystemLocked>(locked(), name(), InputSignal::Name, "CONST(" + std::to_string(value) + ")", "OUT"); 

      signal_t const *ptr = &value;
      constexpr size_t inputIndex = index_of<InputSignal>;

      if (connected(inputIndex, ptr)) return;
      inputState[inputIndex].push_back({ptr, false});
    }

    template <typename S>
    void setOutput(signal_t value) {
      static_assert(S::IsOutput,
		    "Signal passed to setOutput is not an output signal.");
      
      setOutput(index_of<S>, value, S::Mask);
    }

    void setOutput(std::string const &signalName, signal_t value) {
      Error::throw_runtime_error_if
	<Error::InvalidSignalName>(!nameToOutput.contains(signalName), name(), signalName);

      auto [outputIndex, mask] = nameToOutput[signalName];
      return setOutput(outputIndex, value, mask);
    }
    
    void setOutput(size_t outputIndex, signal_t value, signal_t mask = -1) {
      Error::throw_runtime_error_if
	<Error::OutputChangeNotAllowed>(!setOutputAllowed(), name());

      Error::throw_runtime_error_if
	<Error::IndexOutOfBounds>(outputIndex >= Outputs::N, "output", name(), outputIndex, Outputs::N);

      outputState[outputIndex] = value & mask;
    }

    template <typename S>
    signal_t getOutput() {
      static_assert(S::IsOutput,
		    "Signal passed to getOutput is not an output-signal.");

      return getOutput(index_of<S>, S::Mask);;
    }

    signal_t getOutput(std::string const &signalName) {
      Error::throw_runtime_error_if
	<Error::InvalidSignalName>(!nameToOutput.contains(signalName), name(), signalName);

      auto [outputIndex, mask] = nameToOutput[signalName];
      return getInput(outputIndex, mask);
    }
    
    signal_t getOutput(size_t outputIndex, signal_t mask = -1) {
      Error::throw_runtime_error_if
	<Error::IndexOutOfBounds>(outputIndex >= Outputs::N, "output", name(), outputIndex, Outputs::N);
      
      return outputState[outputIndex] & mask;
    }
  
    template <typename S>
    signal_t getInput() {
      static_assert(S::IsInput,
		    "Signal passed to getInput is not an input-signal.");
      
      return getInput(index_of<S>, S::Mask);
    }

    signal_t getInput(std::string const &signalName) {
      Error::throw_runtime_error_if
	<Error::InvalidSignalName>(!nameToInput.contains(signalName), name(), signalName);
      
      auto const [inputIndex, mask] = nameToInput[signalName];
      return getInput(inputIndex, mask);
    }
    
    signal_t getInput(size_t inputIndex, signal_t mask = -1) {
      Error::throw_runtime_error_if
	<Error::IndexOutOfBounds>(inputIndex >= Inputs::N, "input", name(), inputIndex, Inputs::N);
      
      signal_t result = 0;
      for (auto const &[ptr, activeLow]: inputState[inputIndex]) {
	if (ptr) result |= (activeLow ? ~(*ptr) : *ptr);
      }
      return result & mask;
    }
  };


  class System;
  
  class VcdScope {
    
    struct SignalLog {
      std::string mod;
      std::string name;
      signal_t const *ptr;
      signal_t mask;
      std::vector<std::pair<size_t, signal_t>> history;
    };

    std::vector<SignalLog> _monitoredSignals;
    System const &_sys;
    std::string _name;
    
    
  public:
    VcdScope(System const &sys, std::string const &name):
      _sys(sys),
      _name(name)
    {
      monitorClock();
    }

    struct VCD {
      struct Event {
	std::string id;
	size_t time;
	signal_t value;
	size_t width;
      };

      std::vector<Event> events;
      std::string definitions;
    };
    
    // Monitor entire module (all outputs)
    template <typename ModuleType>
    void monitor(ModuleType const &mod);

    // Monitor single output
    template <typename OutputSignal, typename ModuleType>
    void monitor(ModuleType const &mod);

    // Monitor multiple outputs
    template <typename OutputSignal1, typename OutputSignal2, typename ... OutputSignalRest, typename ModuleType>
    void monitor(ModuleType const &mod);

    void sample(size_t time) {
      for (SignalLog &log: _monitoredSignals) {
	if (log.history.empty() || *log.ptr != log.history.back().second) {
	  log.history.push_back({time, *log.ptr & log.mask});
	}
      }
    }

    VCD vcd() const {
      static auto const numberOfBits = [](signal_t mask) -> size_t {
	size_t result = 0;
	while (mask) {
	  ++result;
	  mask >>= 1;
	}
	return result;
      };

      auto const toId = [&](std::string const &modName, std::string const &signalName) -> std::string {
	std::string result;
	for (char c: modName) {
	  if (std::isspace(c) || c == '$' || c == '#') continue;
	  result += std::tolower(c);
	}
	result += '_';
	for (char c: signalName) {
	  if (std::isspace(c) || c == '$' || c == '#') continue;
	  result += std::tolower(c);
	}
	  
	return result;
      };

      std::vector<VCD::Event> events;
      for (SignalLog const &log: _monitoredSignals) {
	for (auto [time, value]: log.history) {
	  events.emplace_back(toId(log.mod, log.name), time, value, numberOfBits(log.mask));
	}
      }

      std::ostringstream out;
      out << "$scope module " << _name << " $end\n";
      for (SignalLog const &log : _monitoredSignals) {
	std::string id = toId(log.mod, log.name);
	out << "$var wire " << numberOfBits(log.mask) << ' '
	    << id << " " << id << " $end\n";
      }
      out << "$upscope $end\n";
      out << "$enddefinitions $end\n\n";

      return {events, out.str()};
    }

    std::string const &name() const { return _name; }
    
  private:
    void monitor(signal_t const *ptr, signal_t mask, std::string const &modName, std::string const &sigName) {
      for (SignalLog const &log: _monitoredSignals) {
	if (log.ptr == ptr) return;
      }
      _monitoredSignals.emplace_back(modName, sigName, ptr, mask);
    }

    void monitorClock();
  };
  
  
  RINKU_INPUT(SYS_HLT, 1);
  RINKU_INPUT(SYS_ERR, 1);
  RINKU_INPUT(SYS_EXIT, 1);
  RINKU_INPUT(SYS_EXIT_CODE, 8);
  RINKU_SIGNAL_LIST(SystemInputSignals, SYS_HLT, SYS_ERR, SYS_EXIT, SYS_EXIT_CODE);

  class System: public Module<SystemInputSignals> {
    friend class VcdScope;
    
    class Clock_ {
      std::vector<std::shared_ptr<Impl::ModuleBase>> _attached;
      signal_t _value = 0;
      
    public:
      void rise() {
	_value = 1;
	for (auto const &m: _attached) {
	  m->resetGuaranteed();
	  m->allowSetOutput(false);
	  m->clockRising();
	  m->allowSetOutput(true);
	}
      }
      
      void fall() {
	_value = 0;
	for (auto const &m: _attached) {
	  m->resetGuaranteed();
	  m->allowSetOutput(false);
	  m->clockFalling();
	  m->allowSetOutput(true);
	}
      }

      template <typename ModuleT>
      void attach(std::shared_ptr<ModuleT> const &m) {
	_attached.emplace_back(m);
      }

      signal_t const &value() const {
	return _value;
      }
    };

    // TODO: prefix members with underscore, check other classes too
    std::unordered_map<std::string, size_t> moduleIndexByName;
    std::vector<std::shared_ptr<Impl::ModuleBase>> modules;
    std::vector<std::shared_ptr<VcdScope>> scopes;
    
    Clock_ clk;
    bool initialized = false;
    size_t moduleCount = 0;
    size_t tickCount = 0;
    double scopeFreq = 0;

    static constexpr char const *ANONYMOUS = "__rinku_anonymous";
    
  public:
    System(double freq = 1):
      scopeFreq(freq)
    {
      setModuleIndex(-2);

      static constexpr double MAX_FREQ = 0.5e12;
      static constexpr double MIN_FREQ = 0.5e-2;
      Error::throw_runtime_error_if
	<Error::SystemFrequencyOutOfRange>(freq > MAX_FREQ || freq < MIN_FREQ, freq, MIN_FREQ, MAX_FREQ);
    }

    VcdScope &addScope(std::string const &name) {
      for (auto const &ptr: scopes) {
	Error::throw_runtime_error_if
	  <Error::DuplicateScopeNames>(name == ptr->name(), name);
      }

      auto ptr = std::make_shared<VcdScope>(*this, name);
      scopes.emplace_back(ptr);
      return *ptr;
    }

    VcdScope const &getScope(std::string const &name) {
      for (auto const &s: scopes) {
	if (s->name() == name) return *s;
      }

      Error::throw_runtime_error<Error::InvalidScopeName>(name);
      UNREACHABLE__;
    }

  private:
    template <typename ModuleT, typename... Args>
    ModuleT& addModuleImpl(std::string const &name, Args&&... args) {
      static_assert(std::is_base_of_v<Impl::ModuleBase, ModuleT>,
		    "Module-type must derive from Module<...>.");

      auto ptr = std::make_shared<ModuleT>(std::forward<Args>(args)...);
      if (name != ANONYMOUS) {

	Error::throw_runtime_error_if
	  <Error::DuplicateModuleNames>(moduleIndexByName.contains(name), name);

	moduleIndexByName[name] = moduleCount;
	ptr->setName(name);
      }
      
      ptr->setModuleIndex(moduleCount);
      clk.attach(ptr);
      modules.emplace_back(ptr);
      ++moduleCount;
      return *ptr;
    }

  public:
    template <typename ModuleT>
    ModuleT& addModule() {
      return addModuleImpl<ModuleT>(ANONYMOUS);
    }
    
    template <typename ModuleT, typename First, typename ... Rest>
    ModuleT& addModule(First&& first, Rest&& ... rest) {
      if constexpr (std::is_convertible_v<First, std::string>) {
	if constexpr (!std::is_constructible_v<ModuleT, First, Rest ...>) {
	  // First arg is a string and object cannot be built including that string -> string is name
	  return addModuleImpl<ModuleT>(std::forward<First>(first),
					std::forward<Rest>(rest)...);
	}
	else {
	  // First arg is a string and object can be built including that string.
	  // Check if it can be built without.
	  if constexpr (!std::is_constructible_v<ModuleT, Rest ...>) {
	    // Can't be built without -> string is part of constructor -> unnamed
	    return addModuleImpl<ModuleT>(ANONYMOUS,
					  std::forward<First>(first),
					  std::forward<Rest>(rest) ...);
	  }
	  else {
	    // Ambiguous
	    static_assert(
			  !sizeof(First), 
			  "Ambiguous addModule<ModuleT>(…): the first argument is both\n"
			  " 1) convertible to std::string (so could be a module-name), and\n"
			  " 2) usable as ModuleT’s first constructor argument.\n"
			  "Please disambiguate by calling either:\n"
			  "  • addModuleUnnamed<ModuleT>(…);   // force unnamed module\n"
			  "  • addModuleNamed<ModuleT>(name, …); // force named module\n"
			  );
	  }
	}
      }
      else {
	return addModuleImpl<ModuleT>(ANONYMOUS,
				      std::forward<First>(first),
				      std::forward<Rest>(rest)...);
      }
    }

    template <typename ModuleT, typename... Args>
    ModuleT& addModuleNamed(std::string const &name, Args&&... args) {
      return addModuleImpl<ModuleT>(name, std::forward<Args>(args)...);
    }

    template <typename ModuleT, typename... Args>
    ModuleT& addModuleUnnamed(Args&&... args) {
      return addModuleImpl<ModuleT>(ANONYMOUS, std::forward<Args>(args)...);
    }
    
    template <typename ModuleT>
    ModuleT &getModule(std::string const &name) {
      Error::throw_runtime_error_if
	<Error::InvalidModuleName>(!moduleIndexByName.contains(name), name);
      
      auto basePtr = modules[moduleIndexByName[name]];
      auto ptr = std::dynamic_pointer_cast<ModuleT>(basePtr);

      Error::throw_runtime_error_if
	<Error::InvalidModuleType>(!ptr, name, typeid(ModuleT).name());
      
      return *ptr;
    }

    template <typename HaltSignal, typename Module_>
    void connectHalt(Module_ &m) {
      connect<SYS_HLT, HaltSignal>(m);
    }

    template <typename ErrorSignal, typename Module_>
    void connectError(Module_ &m) {
      connect<SYS_ERR, ErrorSignal>(m);
    }

    template <typename ExitSignal, typename Module_>
    void connectExit(Module_ &m) {
      connect<SYS_EXIT, ExitSignal>(m);
    }

    template <typename ExitCodeSignal, typename Module_>
    void connectExitCode(Module_ &m) {
      connect<SYS_EXIT_CODE, ExitCodeSignal>(m);
    }

  private:    
    void updateAll() {
      static std::vector<int> q;
      static std::vector<bool> inQ(moduleCount);

      // Populate the queue with all modules
      q.resize(moduleCount);
      std::iota(q.begin(), q.end(), 0);
      std::fill(inQ.begin(), inQ.end(), true);

      // Update and follow the affected modules until system has settled
      size_t qHead = 0;
      while (qHead < q.size()) {
	int idx = q[qHead++];
	inQ[idx] = false;

	std::vector<int> affectedIndices = modules[idx]->updateAndCheck();
	for (int outIdx: affectedIndices) {
	  if (outIdx < 0 || inQ[outIdx]) continue;
	  q.push_back(outIdx);
	  inQ[outIdx] = true;
	}
      }
    }

  public:
    virtual void reset() override {
      for (auto const &m: modules) {
	m->reset();
	m->resetGuaranteed();
      }
      updateAll();
    }

    void init() {
      this->lock();
      for (auto const &m: modules) {
	m->lock();
      }
      reset();
      initialized = true;
    }
    
    signal_t run(bool resume = false) {
      while (step(resume)) {}
      return getInput<SYS_EXIT_CODE>();
    }

    bool halfStep(bool resume = false) {
      checkIfInitialized();

      static bool phase = 0;
      updateAll();
      if (phase == 0) {
	if (getInput<SYS_EXIT>()) {
	  return false;
	}
	if (getInput<SYS_ERR>()) {
	  signal_t err = getInput<SYS_EXIT_CODE>();
	  std::cerr << "\nThe ERR signal was asserted (error code " << err <<  ").\n";
	  return false;
	}
	if (getInput<SYS_HLT>() && !resume) {
	  std::cerr << "\nSystem halted, press any key to resume ...";
	  std::getchar();
	}
	clk.rise();
      }
      else {
	clk.fall();
      }

      phase = !phase;
      updateAll();

      for (auto &scope: scopes) {
	scope->sample(tickCount++);
      }
      
      return true;
    }
    
    bool step(bool resume = false) {
      if (!halfStep(resume)) return false;
      return halfStep(resume);
    }
    
    template <typename ... Scopes>
    std::string vcd(Scopes const & ... _scopes) {

      std::vector<VcdScope const *> scopeVec;
      if constexpr (sizeof ...(Scopes) > 0) {
	static_assert(( ... && (std::is_same_v<std::decay_t<Scopes>, VcdScope> || std::is_convertible_v<std::decay_t<Scopes>, std::string>)),
		      "System::vcd() must be called with either VcdScope objects or strings (names).");
	
	// Build table of scope pointers; arguments could be type VcdScope or strings
	auto const getScopePointer = [&]<typename T>(T const &scopeVar) -> VcdScope const *{
	  if constexpr (std::is_same_v<std::decay_t<T>, VcdScope>) {
	    return &scopeVar;
	  }
	  else {
	    return &getScope(scopeVar);
	  }
	};

	scopeVec = { getScopePointer(_scopes) ... };
      }
      else {
	for (auto const &ptr: scopes) {
	  scopeVec.push_back(ptr.get());
	}
      }
      
      auto const getTimescale = [](double f) -> std::string {
	static constexpr char const *values[] = { "100", "10", "1" };
	static constexpr char const *units[] = { "s", "ms", "us" ,"ns", "ps" };
	int const power = std::round(std::log10(2 * f)) + 2;
	return (std::string(values[power % 3]) + std::string(units[ power / 3]));
      };

      // Emit header
      std::ostringstream out;
      auto now = std::chrono::system_clock::now();
      auto zoned = std::chrono::zoned_time(std::chrono::current_zone(), now);
      std::string currentDateStr = std::format("{:%Y-%m-%d %H:%M}", zoned);      
      out << "$date " << currentDateStr << " $end\n";
      out << "$version Rinku VCD Dump $end\n";
      out << "$timescale " << getTimescale(scopeFreq) << " $end\n\n";

      // Emit definitions for all scopes and collect events
      std::vector<VcdScope::VCD::Event> events;
      for (VcdScope const *scope: scopeVec) {
	VcdScope::VCD currentVcd = scope->vcd();
	out << currentVcd.definitions;
	events.insert(events.end(), currentVcd.events.begin(), currentVcd.events.end());
      }
      std::sort(events.begin(), events.end(), [](auto const &a, auto const &b) {
	return a.time < b.time;
      });

      // Emit value changes
      size_t currentTime = -1;
      for (auto const &event: events) {
	if (event.time != currentTime) {
	  currentTime = event.time;
	  out << '#' << currentTime << "\n";
	}
	std::string bits = std::bitset<64>(event.value).to_string();
	out << 'b' << bits.substr(bits.length() - event.width) << ' ' << event.id << '\n';
      }
      
      return out.str();
    };
    
  private:
    template <typename S, typename ModuleType>
    signal_t const *getOutputSignalPointer(ModuleType const &mod) const {
      return getOutputSignalPointer(mod, ModuleType::template index_of<S>);
    }

    template <typename ModuleType>
    signal_t const *getOutputSignalPointer(ModuleType const &mod, size_t index) const {
      
      Error::throw_runtime_error_if
	<Error::InvalidModuleName>(!moduleIndexByName.contains(mod.name()), mod.name());

      Error::throw_runtime_error_if
	<Error::IndexOutOfBounds>(index >= ModuleType::Outputs::N,
				  "output", mod.name(), index, ModuleType::Outputs::N);
      
      return &mod.outputState[index];
    }

    signal_t const *getClockSignalPointer() const {
      return &clk.value();
    }
    
    void checkIfInitialized() {
      if (!initialized) Error::throw_runtime_error<Error::SystemNotInitialized>();
    }
  };


  template <typename OutputSignal1, typename OutputSignal2, typename ... OutputSignalRest, typename ModuleType>
  void VcdScope::monitor(ModuleType const &mod) {
    monitor<OutputSignal1>(mod);
    monitor<OutputSignal2>(mod);
    (monitor<OutputSignalRest>(mod), ...);
  }
  
  template <typename OutputSignal, typename ModuleType>
  void VcdScope::monitor(ModuleType const &mod) {
    static_assert(OutputSignal::IsOutput, "VcdScope can only monitor output signals");

    Error::throw_runtime_error_if
      <Error::SystemLocked>(_sys.locked(), _name, "SCOPE_IN", mod.name(), OutputSignal::Name);

    signal_t const *ptr = _sys.getOutputSignalPointer<OutputSignal>(mod);
    assert(ptr && "getOutputSignalPointer should never return nullptr");
    
    monitor(ptr, OutputSignal::Mask, mod.name(), OutputSignal::Name);
  }

  template <typename ModuleType>
  void VcdScope::monitor(ModuleType const &mod) {
    char const * const *outputNames = ModuleType::Outputs::names();
    signal_t const *outputMasks = ModuleType::Outputs::masks();
    
    for (size_t idx = 0; idx != ModuleType::Outputs::N; ++idx) {
      monitor(_sys.getOutputSignalPointer(mod, idx), outputMasks[idx], mod.name(), outputNames[idx]);
    }
  }

  inline void VcdScope::monitorClock() {
    monitor(_sys.getClockSignalPointer(), 1, "System", "CLK");
  }

  
} // namespace Rinku

#endif // RINKU_H
