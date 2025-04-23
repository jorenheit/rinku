#ifndef RINKU_H
#define RINKU_H

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <cassert>
#include <cstdint>
#include <algorithm>
#include <memory>
#include <iostream>

#define RINKU_INPUT(SIGNAL, N) struct SIGNAL: Rinku::Impl::Input_<N, SIGNAL> {};
#define RINKU_OUTPUT(SIGNAL, N) struct SIGNAL: Rinku::Impl::Output_<N, SIGNAL> {};
#define RINKU_SIGNAL_LIST(NAME, ...) using NAME = Rinku::Impl::Signals_< __VA_ARGS__ >;
#define RINKU_MODULE(...) public Rinku::Module< __VA_ARGS__ >
#define RINKU_ON_CLOCK_RISING() virtual void clockRising() override
#define RINKU_ON_CLOCK_FALLING() virtual void clockFalling() override
#define RINKU_UPDATE() virtual void update() override
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

#define RINKU_ADD_MODULE1(MODULE_TYPE) addModule<MODULE_TYPE>
#define RINKU_ADD_MODULE2(SYSTEM, MODULE_TYPE) (SYSTEM).addModule<MODULE_TYPE>
#define RINKU_ADD_MODULE(...)	      \
  RINKU_PP_GET_2ND(__VA_ARGS__,	      \
		   RINKU_ADD_MODULE2, \
		   RINKU_ADD_MODULE1) \
    (__VA_ARGS__)

#define RINKU_CONNECT_MOD(INPUT_OBJECT, INPUT_SIGNAL, OUTPUT_OBJECT, OUTPUT_SIGNAL) \
  (INPUT_OBJECT).connect<INPUT_SIGNAL, OUTPUT_SIGNAL>((OUTPUT_OBJECT), { \
      __FILE__,								\
      __LINE__,								\
      #INPUT_OBJECT,							\
      #INPUT_SIGNAL,							\
      #OUTPUT_OBJECT,							\
      #OUTPUT_SIGNAL							\
    })

#define RINKU_CONNECT_CONST(INPUT_OBJECT, INPUT_SIGNAL, VALUE)	\
  (INPUT_OBJECT).connect<INPUT_SIGNAL, VALUE>({			\
      __FILE__,							\
      __LINE__,							\
      #INPUT_OBJECT,						\
      #INPUT_SIGNAL,						\
      #VALUE,							\
      "CONST"							\
    })

#define RINKU_GET_OUTPUT(OUTPUT_SIGNAL)		\
  getOutput<OUTPUT_SIGNAL>({			\
      __FILE__,					\
      __LINE__,					\
      "None",					\
      "None",					\
      "this",					\
      #OUTPUT_SIGNAL				\
    })

#define RINKU_GET_OUTPUT_INDEX(INDEX)		\
  getOutput((INDEX), -1, {			\
      __FILE__,					\
      __LINE__,					\
      "None",					\
      "None",					\
      "this",					\
      #INDEX					\
    })

#define RINKU_SET_OUTPUT(OUTPUT_SIGNAL, VALUE)		\
  setOutput<OUTPUT_SIGNAL>((VALUE), {			\
      __FILE__,						\
      __LINE__,						\
      "None",						\
      "None",						\
      "this",						\
      #OUTPUT_SIGNAL					\
    })

#define RINKU_SET_OUTPUT_INDEX(INDEX, VALUE)		\
  setOutput((INDEX), (VALUE), -1, {			\
      __FILE__,						\
      __LINE__,						\
      "None",						\
      "None",						\
      "this",						\
      #INDEX						\
    })

#define RINKU_GET_INPUT(INPUT_SIGNAL)			\
  getInput<INPUT_SIGNAL>({				\
      __FILE__,						\
      __LINE__,						\
      "this",						\
      #INPUT_SIGNAL,					\
      "None",						\
      "None"						\
    })

#define RINKU_GET_INPUT_INDEX(INDEX)			\
  getInput((INDEX), -1, {				\
      __FILE__,						\
      __LINE__,						\
      "this",						\
      #INDEX,						\
      "None",						\
      "None"						\
    })


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
#endif

namespace Rinku {

  using signal_t = uint64_t;
  
  namespace Impl {

    template <typename ... Args>
    void runtime_msg(std::string const &pre, Args ... args) {
      (std::cerr << pre << ": " << ... << args) << '\n'; 
    }
    
    template <typename ... Args>
    void runtime_error(Args ... args) {
      runtime_msg("ERROR", args...);
      std::exit(1);
    }

    template <typename ... Args>
    void runtime_error_if(bool condition, Args ... args) {
      if (condition) runtime_error(args ...);
    }

    template <typename ... Args>
    void runtime_warning(Args ... args) {
      runtime_msg("WARNING", args...);
    }

    template <typename ... Args>
    void runtime_warning_if(bool condition, Args ... args) {
      if (condition) runtime_warning(args...);
    }
        
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
      template <typename S>
      static consteval size_t index_of_() {
	constexpr bool match[] = { std::is_same_v<S, Args> ... };
	for (size_t idx = 0; idx != sizeof ... (Args); ++idx) {
	  if (match[idx]) return idx;
	}
	return -1;
      }

    public:
      static constexpr size_t N = sizeof ... (Args);
  
      template <typename S>
      static consteval size_t index_of() {
	constexpr size_t index = index_of_<S>();
	static_assert(index != -1, "Provided signal is not part of the list for this module.");
	return index;
      }

      static consteval bool is_input_list() {
	constexpr bool isInput[] = { (Args::IsInput) ... };
	for (size_t idx = 0; idx != sizeof ... (Args); ++idx) {
	  if (!isInput[idx]) return false;
	}
	return true;
      }

      static consteval bool is_output_list() {
	constexpr bool isOutput[] = { (Args::IsOutput) ... };
	for (size_t idx = 0; idx != sizeof ... (Args); ++idx) {
	  if (!isOutput[idx]) return false;
	}
	return true;
      }

      static_assert(is_input_list() || is_output_list(),
		    "Signal list must contain only inputs or outputs.");
    };

    class ModuleBase {
      bool _locked = false;
      bool _setOutputAllowed = true;
      std::vector<bool> _isUpdating;
      std::vector<std::unordered_set<ModuleBase*>> _dependencies;
      
    public:
      ModuleBase(size_t nInputs):
	_isUpdating(nInputs),
	_dependencies(nInputs)
      {}
      
      virtual ~ModuleBase() = default;
      virtual void clockRising() {}
      virtual void clockFalling() {}
      virtual void update() {}
      virtual void reset() {}

      void addDependency(size_t inputIndex, ModuleBase &dep) {
	_dependencies[inputIndex].insert(&dep);
      }

      void updateDependencies(size_t inputIndex) {
	if (_isUpdating[inputIndex]) return;
	_isUpdating[inputIndex] = true;
	
	for (ModuleBase *dep: _dependencies[inputIndex]) {
	  dep->allowSetOutput(true);
	  dep->update();
	  dep->allowSetOutput(false);
	}

	_isUpdating[inputIndex] = false;
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
    };

    struct DebugInfo {
      char const *file = nullptr;
      int line = 0;
      char const *inputObject = nullptr;
      char const *inputSignal = nullptr;
      char const *outputObject = nullptr;
      char const *outputSignal = nullptr;

      operator bool() const {
	return valid();
      }
      
      bool valid() const {
	if (!file) return false;
	if (!line) return false;
	if (!inputObject) return false;
	if (!inputSignal) return false;
	if (!outputObject) return false;
	if (!outputSignal) return false;
	return true;
      }
    };
  } // namespace Impl


  template <typename Output>
  struct Not: Impl::Output_<Output::Width, typename Output::Base, !Output::ActiveLow>
  {};
  
  template <typename SignalList1 = Impl::Signals_<>,
	    typename SignalList2 = Impl::Signals_<>>
  class Module: public Impl::ModuleBase {

    template <typename, typename>
    friend class Module;

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
    std::unordered_set<signal_t> constants;
    signal_t outputState[Outputs::N] {};
    std::unordered_set<signal_t const *> inputState[Inputs::N] {};
    std::unordered_map<signal_t const *, bool> activeLow[Inputs::N] {};

    using ModuleBase::allowSetOutput;
    using ModuleBase::setOutputAllowed;
    
  public:
    Module():
      Impl::ModuleBase(Inputs::N)
    {}
    
    template <typename InputSignal, typename OutputSignal, typename OtherModule>
    void connect(OtherModule &other, Impl::DebugInfo const &dbg = {}) {
      static_assert(std::is_base_of_v<Impl::ModuleBase, OtherModule>,
		    "Other module is not a valid Rinku::Module.");

      Impl::runtime_error_if(dbg && locked(),
			     dbg.file, ":", dbg.line, ": connect called on module \"", dbg.inputObject, "\" "
			     "after calling System::init().");
      
      Impl::runtime_error_if(!dbg && locked(),
			     "Cannot connect signal \"", typeid(InputSignal).name(),
			     "\" after System::init().");
    
      constexpr size_t inputIndex = index_of<InputSignal>;
      constexpr size_t outputIndex = OtherModule::template index_of<typename OutputSignal::Base>;

      signal_t *ptr = &other.outputState[outputIndex];
      bool const signalAlreadyConnected = inputState[inputIndex].contains(ptr);
      
      Impl::runtime_warning_if(dbg && signalAlreadyConnected,
			       dbg.file, ":", dbg.line, ": Signal \"", dbg.inputSignal, "\" of module \"",
			       dbg.inputObject, "\" is already connected to output \"", dbg.outputSignal,
			       "\" of module \"", dbg.outputObject, "\".");

      Impl::runtime_warning_if(!dbg && signalAlreadyConnected,
			       "Signal \"", typeid(OutputSignal).name(), "\" is already connected to \"",
			       typeid(InputSignal).name(), "\" of module \"", typeid(OtherModule).name(), "\".");

      addDependency(inputIndex, other);
      inputState[inputIndex].insert(ptr);
      activeLow[inputIndex][ptr] = OutputSignal::ActiveLow;
    }

    template <typename InputSignal, signal_t Value>
    void connect(Impl::DebugInfo const &dbg = {}) {
      static_assert(Value < (1 << InputSignal::Width),
		    "Constant input value too large for signal-width.");
      
      Impl::runtime_error_if(dbg && locked(),
			     dbg.file, ":", dbg.line, ": connect called on module \"", dbg.inputObject, "\" "
			     "after calling System::init().");
      Impl::runtime_error_if(!dbg && locked(),
			     "Cannot connect signal \"", typeid(InputSignal).name(),
			     "\" after System::init().");

      signal_t const *ptr = &(*(constants.insert(Value).first));
      constexpr size_t inputIndex = index_of<InputSignal>;
      bool const signalAlreadyConnected = inputState[inputIndex].contains(ptr);

      Impl::runtime_warning_if(dbg && signalAlreadyConnected,
			       dbg.file, ":", dbg.line, ": Signal \"", dbg.inputSignal, "\" of module \"",
			       dbg.inputObject, "\" is already connected to constant \"", Value ,"\".");
      Impl::runtime_warning_if(!dbg && signalAlreadyConnected,
			       "Signal \"", typeid(InputSignal).name(), "\" is already connected to constant \"",
			       Value, "\".");
      
      inputState[inputIndex].insert(ptr);
      activeLow[inputIndex][ptr] = false;
    }

    template <typename S>
    void setOutput(signal_t value, Impl::DebugInfo const &dbg = {}) {
      static_assert(S::IsOutput,
		    "Signal passed to setOutput is not an output signal.");
      
      setOutput(index_of<S>, value, S::Mask, dbg);
    }

    void setOutput(size_t outputIndex, signal_t value, signal_t mask = -1, Impl::DebugInfo const &dbg = {}) {
      Impl::runtime_error_if(dbg && !setOutputAllowed(),
		       dbg.file, ":", dbg.line,
		       ": Outputs should be modified in the update-function, not the clock-handlers.");
      Impl::runtime_error_if(!dbg && !setOutputAllowed(),
		       "Outputs should be modified in the update-function, not the clock-handlers.");
      Impl::runtime_error_if(dbg && outputIndex >= Outputs::N,
		       dbg.file, ":", dbg.line,
		       ": Output-index (", outputIndex, ") out of bounds (#output-signals = ", Outputs::N ,").");
      Impl::runtime_error_if(!dbg && outputIndex >= Outputs::N,
		       "Output-index (", outputIndex, ") out of bounds (#output-signals = ", Outputs::N ,").");

      outputState[outputIndex] = value & mask;
    }

    template <typename S>
    signal_t getOutput(Impl::DebugInfo const &dbg = {}) {
      static_assert(S::IsOutput,
		    "Signal passed to getOutput is not an output-signal.");

      return getOutput(index_of<S>, S::Mask, dbg);;
    }

    signal_t getOutput(size_t outputIndex, signal_t mask = -1, Impl::DebugInfo const &dbg = {}) {
      Impl::runtime_error_if(dbg && outputIndex >= Outputs::N,
		       dbg.file, ":", dbg.line,
		       ": Output-index (", outputIndex, ") out of bounds (#output-signals = ", Outputs::N ,").");
      Impl::runtime_error_if(!dbg && outputIndex >= Outputs::N,
		       "Output-index (", outputIndex, ") out of bounds (#output-signals = ", Outputs::N ,").");
      
      return outputState[outputIndex] & mask;
    }
  
    template <typename S>
    signal_t getInput(Impl::DebugInfo const &dbg = {}) {
      static_assert(S::IsInput,
		    "Signal passed to getInput is not an input-signal.");
      
      return getInput(index_of<S>, S::Mask, dbg);
    }

    signal_t getInput(size_t inputIndex, signal_t mask = -1, Impl::DebugInfo const &dbg = {}) {
      updateDependencies(inputIndex);
      
      Impl::runtime_error_if(dbg && inputIndex >= Inputs::N,
		       dbg.file, ":", dbg.line,
		       ": Input-index (", inputIndex, ") out of bounds (#input-signals = ", Inputs::N ,").");
      Impl::runtime_error_if(!dbg && inputIndex >= Inputs::N,
		       "Input-index (", inputIndex, ") out of bounds (#input-signals = ", Inputs::N ,").");

      signal_t result = 0;
      for (signal_t const *ptr: inputState[inputIndex]) {
	assert(activeLow[inputIndex].contains(ptr) &&
	       "FATAL BUG: activeLow does not contain the ptr. Please report this bug.");
	
	if (ptr) result |= (activeLow[inputIndex][ptr] ? ~(*ptr) : *ptr);
      }
      return result & mask;
    }
  };

  RINKU_INPUT(SYS_HLT, 1);
  RINKU_INPUT(SYS_ERR, 1);
  RINKU_INPUT(SYS_EXIT, 1);
  RINKU_INPUT(SYS_EXIT_CODE, 8);
  RINKU_SIGNAL_LIST(SystemInputSignals, SYS_HLT, SYS_ERR, SYS_EXIT, SYS_EXIT_CODE);

  class System: public Module<SystemInputSignals> {

    class Clock {
      std::vector<std::shared_ptr<Impl::ModuleBase>> attached;

    public:
      void rise() {
	for (auto const &m: attached) {
	  m->clockRising();
	}
      }
      
      void fall() {
	for (auto const &m: attached) {
	  m->clockFalling();
	}
      }

      template <typename ModuleT>
      void attach(std::shared_ptr<ModuleT> const &m) {
	attached.emplace_back(m);
      }
    };
    
    std::vector<std::shared_ptr<Impl::ModuleBase>> modules;
    Clock clk;
    bool initialized = false;
  
  public:
    System() = default;
    
    template <typename ModuleT, typename... Args>
    ModuleT& addModule(Args&&... args) {
      static_assert(std::is_base_of_v<Impl::ModuleBase, ModuleT>,
		    "Module-type must derive from Module<...>.");
      
      auto ptr = std::make_shared<ModuleT>(std::forward<Args>(args)...);
      clk.attach(ptr);
      modules.emplace_back(ptr);  
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
  
    void init() {
      this->lock();
      for (auto const &m: modules) {
	m->lock();
	m->reset();
      }
      initialized = true;

    }

    virtual void reset() override {
      for (auto const &m: modules)
	m->reset();
    }
    
    signal_t run(bool resume = false) {
      checkIfInitialized();
      while (!getInput<SYS_EXIT>()) step(resume);
      return getInput<SYS_EXIT_CODE>();
    }

    void step(bool resume = false) {
      checkIfInitialized();

      if (getInput<SYS_ERR>()) {
	signal_t err = getInput<SYS_EXIT_CODE>();
	Impl::runtime_error("The ERR signal was asserted (error code ", err, ").");
      }

      if (getInput<SYS_HLT>() && !resume) {
	std::cout << "\nSystem halted, press any key to resume ...";
	std::getchar();
      }

      clk.rise();
      clk.fall();
    }
  private:
    void checkIfInitialized() {
      Impl::runtime_error_if(!initialized,
			     "System not initialized. Call System::init() before calling System::run().");
    }
  };
  
} // namespace Rinku

#endif // RINKU_H
