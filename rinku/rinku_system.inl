
inline void System::Clock_::rise() {
  _value = 1;
  for (auto const &m: _attached) {
    m->resetGuaranteed();
    m->allowSetOutput(false);
    m->clockRising();
    m->allowSetOutput(true);
  }
}

inline void System::Clock_::fall() {
  _value = 0;
  for (auto const &m: _attached) {
    m->resetGuaranteed();
    m->allowSetOutput(false);
    m->clockFalling();
    m->allowSetOutput(true);
  }
}

inline signal_t const &System::Clock_::value() const {
  return _value;
}

template <typename ModuleT>
void System::Clock_::attach(std::shared_ptr<ModuleT> const &m) {
  _attached.emplace_back(m);
}


inline System::System(double freq):
  _scopeFreq(freq)
{
  setModuleIndex(-2);

  static constexpr double MAX_FREQ = 0.5e12;
  static constexpr double MIN_FREQ = 0.5e-2;
  Error::throw_runtime_error_if
    <Error::SystemFrequencyOutOfRange>(freq > MAX_FREQ || freq < MIN_FREQ, freq, MIN_FREQ, MAX_FREQ);
}

inline VcdScope &System::addScope(std::string const &name) {
  for (auto const &ptr: _scopes) {
    Error::throw_runtime_error_if
      <Error::DuplicateScopeNames>(name == ptr->name(), name);
  }

  auto ptr = std::make_shared<VcdScope>(*this, name);
  _scopes.emplace_back(ptr);
  return *ptr;
}

inline VcdScope const &System::getScope(std::string const &name) {
  for (auto const &s: _scopes) {
    if (s->name() == name) return *s;
  }

  Error::throw_runtime_error<Error::InvalidScopeName>(name);
  UNREACHABLE__;
}

template <typename ModuleT, typename... Args>
ModuleT& System::addModuleImpl(std::string const &name, Args&&... args) {
  static_assert(std::is_base_of_v<Impl::ModuleBase, ModuleT>,
		"Module-type must derive from Module<...>.");

  auto ptr = std::make_shared<ModuleT>(std::forward<Args>(args)...);
  if (name != ANONYMOUS) {

    Error::throw_runtime_error_if
      <Error::DuplicateModuleNames>(_moduleIndexByName.contains(name), name);

    _moduleIndexByName[name] = _moduleCount;
    ptr->setName(name);
  }
      
  ptr->setModuleIndex(_moduleCount);
  _clk.attach(ptr);
  _modules.emplace_back(ptr);
  ++_moduleCount;
  return *ptr;
}

template <typename ModuleT>
ModuleT& System::addModule() {
  return addModuleImpl<ModuleT>(ANONYMOUS);
}
    
template <typename ModuleT, typename First, typename ... Rest>
ModuleT& System::addModule(First&& first, Rest&& ... rest) {
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
ModuleT& System::addModuleNamed(std::string const &name, Args&&... args) {
  return addModuleImpl<ModuleT>(name, std::forward<Args>(args)...);
}

template <typename ModuleT, typename... Args>
ModuleT& System::addModuleUnnamed(Args&&... args) {
  return addModuleImpl<ModuleT>(ANONYMOUS, std::forward<Args>(args)...);
}
    
template <typename ModuleT>
ModuleT &System::getModule(std::string const &name) {
  Error::throw_runtime_error_if
    <Error::InvalidModuleName>(!_moduleIndexByName.contains(name), name);
      
  auto basePtr = _modules[_moduleIndexByName[name]];
  auto ptr = std::dynamic_pointer_cast<ModuleT>(basePtr);

  Error::throw_runtime_error_if
    <Error::InvalidModuleType>(!ptr, name, typeid(ModuleT).name());
      
  return *ptr;
}

inline std::vector<std::string> System::namedModules() const {
  std::vector<std::string> result;
  for (auto const &[name, index]: _moduleIndexByName) {
    result.push_back(name);
  }
  return result;
}

template <typename HaltSignal, typename Module_>
void System::connectHalt(Module_ &m) {
  connect<SYS_HLT, HaltSignal>(m);
}

template <typename ErrorSignal, typename Module_>
void System::connectError(Module_ &m) {
  connect<SYS_ERR, ErrorSignal>(m);
}

template <typename ExitSignal, typename Module_>
void System::connectExit(Module_ &m) {
  connect<SYS_EXIT, ExitSignal>(m);
}

template <typename ExitCodeSignal, typename Module_>
void System::connectExitCode(Module_ &m) {
  connect<SYS_EXIT_CODE, ExitCodeSignal>(m);
}

inline void System::updateAll() {
  static std::vector<int> q;
  static std::vector<bool> inQ(_moduleCount);

  // Populate the queue with all modules
  q.resize(_moduleCount);
  std::iota(q.begin(), q.end(), 0);
  std::fill(inQ.begin(), inQ.end(), true);

  // Update and follow the affected modules until system has settled
  size_t qHead = 0;
  while (qHead < q.size()) {
    int idx = q[qHead++];
    inQ[idx] = false;

    std::vector<int> affectedIndices = _modules[idx]->updateAndCheck();
    for (int outIdx: affectedIndices) {
      if (outIdx < 0 || inQ[outIdx]) continue;
      q.push_back(outIdx);
      inQ[outIdx] = true;
    }
  }
}
    
inline void System::reset() {
  _tickCount = 0;
  for (auto const &m: _modules) {
    m->reset();
    m->resetGuaranteed();
  }
  updateAll();
}

inline void System::init() {
  this->lock();
  for (auto const &m: _modules) {
    m->lock();
  }
  reset();
  _initialized = true;
}
    
inline signal_t System::run(bool resume) {
  while (step(resume)) {}
  return getInput<SYS_EXIT_CODE>();
}

inline bool System::halfStep(bool resume) {
  checkIfInitialized();

  updateAll();
  if ((_tickCount & 1) == 0) {
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
    _clk.rise();
  }
  else {
    _clk.fall();
  }

  updateAll();

  for (auto &scope: _scopes) {
    scope->sample(_tickCount);
  }

  ++_tickCount;
  return true;
}
    
inline bool System::step(bool resume) {
  if (!halfStep(resume)) return false;
  return halfStep(resume);
}
    
template <typename ... Scopes>
std::string System::vcd(Scopes const & ... args) {

  std::vector<VcdScope const *> scopeVec;
  if constexpr (sizeof ...(Scopes) > 0) {
    static_assert(( ... && (std::is_same_v<std::decay_t<Scopes>, VcdScope> || std::is_convertible_v<std::decay_t<Scopes>, std::string>)),
		  "System::vcd() must be called with either VcdScope objects or strings (names).");
	
    scopeVec = {
      [&]<typename T>(T const &scopeVar) -> VcdScope const * {
	if constexpr (std::is_same_v<std::decay_t<T>, VcdScope>) return &scopeVar;
	else return &getScope(scopeVar);
      }(args)
      ...
    };
  }
  else {
    for (auto const &ptr: _scopes) {
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
  out << "$timescale " << getTimescale(_scopeFreq) << " $end\n\n";

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


template <typename S, typename ModuleType>
signal_t const *System::getOutputSignalPointer(ModuleType const &mod) const {
  return getOutputSignalPointer(mod, ModuleType::template index_of<S>);
}

template <typename ModuleType>
signal_t const *System::getOutputSignalPointer(ModuleType const &mod, size_t index) const {

  Error::throw_runtime_error_if
    <Error::InvalidModuleName>(!_moduleIndexByName.contains(mod.ModuleBase::name()), mod.ModuleBase::name());

  Error::throw_runtime_error_if
    <Error::IndexOutOfBounds>(index >= ModuleType::Outputs::N,
			      "output", mod.ModuleBase::name(), index, ModuleType::Outputs::N);
      
  return &mod.outputState[index];
}

inline signal_t const *System::getClockSignalPointer() const {
  return &_clk.value();
}
    
inline void System::checkIfInitialized() {
  if (!_initialized) Error::throw_runtime_error<Error::SystemNotInitialized>();
}
