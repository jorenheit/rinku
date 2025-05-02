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

namespace Rinku {
  using signal_t = uint64_t;

#include "rinku_macros.inl"

  namespace Error {
    
    class Exception {
      std::string _msg;

    public:
      template <typename ... Args>
      Exception(Args&& ... args);    
      std::string const &what() const;
    };
    
    struct SystemLocked;
    struct OutputChangeNotAllowed;
    struct IndexOutOfBounds;  
    struct InvalidSignalName;    
    struct DuplicateModuleNames;  
    struct InvalidModuleName;
    struct InvalidModuleType;
    struct SystemNotInitialized;
    struct InvalidScopeName;
    struct DuplicateScopeNames;
    struct SystemFrequencyOutOfRange;
    
#include "rinku_error.inl"
  }
  
#include "rinku_impl.inl"
#include "rinku_modulebase.inl"

  template <typename Output>
  struct Not: Impl::Output_<Output::Width, typename Output::Base, !Output::ActiveLow> {
    static_assert(Output::IsOutput, "Cannot negate input signals.");
    static constexpr char const *Name = "NOT";
  };

  template <typename SignalList1 = Impl::Signals_<>,
	    typename SignalList2 = Impl::Signals_<>>
  class Module: public Impl::ModuleBase {

    template <typename, typename>
    friend class Module;
    
    friend class System;
    friend class Debugger;
    
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

    virtual size_t nInputs()  const override final { return Inputs::N; }
    virtual size_t nOutputs() const override final { return Outputs::N; }
    
    virtual std::vector<std::string> getInputSignalNames() const override final {
      return std::vector<std::string>(Inputs::names(), Inputs::names() + Inputs::N);
    }

    virtual std::vector<std::string> getOutputSignalNames() const override final {
      return std::vector<std::string>(Outputs::names(), Outputs::names() + Outputs::N);
    }

    template <typename S, typename SignalList = std::conditional_t<S::IsInput, Inputs, Outputs>>
    static constexpr size_t index_of = SignalList::template index_of<S>();

  private:
    signal_t outputState[Outputs::N] {};
    std::vector<int> outputModules[Outputs::N] {};
    std::vector<std::pair<signal_t const*, bool>> inputState[Inputs::N];
    std::unordered_map<std::string, size_t> nameToInput;
    std::unordered_map<std::string, size_t> nameToOutput;
    
  public:
    Module();

    using ModuleBase::name;
    
    template <typename InputSignal, typename OutputSignal, typename OtherModule>
    void connect(OtherModule &other);
    
    template <typename InputSignal, signal_t Value>
    void connect();
    
    template <typename S>
    void setOutput(signal_t value);

    template <typename S>
    signal_t getOutput() const;

    template <typename S>
    signal_t getInput() const;
    
    virtual void setOutput(std::string const &signalName, signal_t value) override final;
    virtual void setOutput(size_t outputIndex, signal_t value) override final;
    virtual signal_t getInput(std::string const &signalName) const override final;
    virtual signal_t getInput(size_t inputIndex) const override final;
    virtual signal_t getOutput(std::string const &signalName) const override final;
    virtual signal_t getOutput(size_t outputIndex) const override final;

  private:
    virtual std::vector<int> updateAndCheck() override final;
    
    void addOutgoing(size_t outputIndex, int idx);
    std::vector<int> const &outgoing(size_t outputIndex);
    bool connected(size_t inputIndex, signal_t const *ptr);
  }; // class Module


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
    
    VcdScope(System const &sys, std::string const &name);

    template <typename ModuleType>
    void monitor(ModuleType const &mod);    // Monitor entire module (all outputs)

    template <typename OutputSignal, typename ModuleType>
    void monitor(ModuleType const &mod);    // Monitor single output

    template <typename OutputSignal1, typename OutputSignal2, typename ... OutputSignalRest, typename ModuleType>
    void monitor(ModuleType const &mod);    // Monitor multiple outputs

    void sample(size_t time);
    VCD vcd() const;
    std::string const &name() const;
    
  private:
    void monitor(signal_t const *ptr, signal_t mask, std::string const &modName, std::string const &sigName);
    void monitorClock();

  }; // class VcdScope
  
  
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
      void rise();
      void fall();
      signal_t const &value() const;

      template <typename ModuleT>
      void attach(std::shared_ptr<ModuleT> const &m);
    };

    std::unordered_map<std::string, size_t> _moduleIndexByName;
    std::vector<std::shared_ptr<Impl::ModuleBase>> _modules;
    std::vector<std::shared_ptr<VcdScope>> _scopes;
    
    Clock_ _clk;
    bool _initialized = false;
    size_t _moduleCount = 0;
    size_t _tickCount = 0;
    double _scopeFreq = 0;

    static constexpr char const *ANONYMOUS = "__rinku_anonymous";
    
  public:
    System(double freq = 1);
    
    VcdScope &addScope(std::string const &name);
    VcdScope const &getScope(std::string const &name);
    
    template <typename ModuleT>
    ModuleT& addModule();
    
    template <typename ModuleT, typename First, typename ... Rest>
    ModuleT& addModule(First&& first, Rest&& ... rest);

    template <typename ModuleT, typename... Args>
    ModuleT& addModuleNamed(std::string const &name, Args&&... args);
    
    template <typename ModuleT, typename... Args>
    ModuleT& addModuleUnnamed(Args&&... args);
    
    template <typename ModuleT>
    ModuleT &getModule(std::string const &name);
    
    template <typename HaltSignal, typename Module_>
    void connectHalt(Module_ &m);

    template <typename ErrorSignal, typename Module_>
    void connectError(Module_ &m);

    template <typename ExitSignal, typename Module_>
    void connectExit(Module_ &m);

    template <typename ExitCodeSignal, typename Module_>
    void connectExitCode(Module_ &m);

    virtual void reset() override final;

    void init();    
    signal_t run(bool resume = false);
    bool halfStep(bool resume = false);    
    bool step(bool resume = false);
    void updateAll();

    template <typename ... Scopes>
    std::string vcd(Scopes const & ... args);

    std::vector<std::string> namedModules() const;
    
#ifdef RINKU_ENABLE_DEBUGGER
    void debug();
#endif
    
  private:
    
    template <typename ModuleT, typename... Args>
    ModuleT& addModuleImpl(std::string const &name, Args&&... args);

    template <typename S, typename ModuleType>
    signal_t const *getOutputSignalPointer(ModuleType const &mod) const;
    
    template <typename ModuleType>
    signal_t const *getOutputSignalPointer(ModuleType const &mod, size_t index) const;
    
    signal_t const *getClockSignalPointer() const;
    
    void checkIfInitialized();

  }; // class System


  // Implementations
  #include "rinku_module.inl"
  #include "rinku_vcdscope.inl"
  #include "rinku_system.inl"
  
} // namespace Rinku

#endif // RINKU_H
