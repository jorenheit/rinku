
inline VcdScope::VcdScope(System const &sys, std::string const &name):
  _sys(sys),
  _name(name)
{}

inline std::string const &VcdScope::name() const {
  return _name;
}


inline void VcdScope::sample(size_t time) {
  for (SignalLog &log: _monitoredSignals) {
    if (log.history.empty() || *log.ptr != log.history.back().second) {
      log.history.push_back({time, *log.ptr & log.mask});
    }
  }
}

inline VcdScope::VCD VcdScope::vcd() const {
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


inline void VcdScope::monitor(signal_t const *ptr, signal_t mask, std::string const &modName, std::string const &sigName) {
  for (SignalLog const &log: _monitoredSignals) {
    if (log.ptr == ptr) return;
  }
  _monitoredSignals.emplace_back(modName, sigName, ptr, mask);
}

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
    <Error::SystemLocked>(_sys.locked(), _name, "SCOPE_IN", mod.ModuleBase::name(), OutputSignal::Name);

  signal_t const *ptr = _sys.getOutputSignalPointer<OutputSignal>(mod);
  assert(ptr && "getOutputSignalPointer should never return nullptr");
    
  monitor(ptr, OutputSignal::Mask, mod.ModuleBase::name(), OutputSignal::Name);
}

template <typename ModuleType>
void VcdScope::monitor(ModuleType const &mod) {
  char const * const *outputNames = ModuleType::Outputs::names();
  signal_t const *outputMasks = ModuleType::Outputs::masks();
    
  for (size_t idx = 0; idx != ModuleType::Outputs::N; ++idx) {
    monitor(_sys.getOutputSignalPointer(mod, idx), outputMasks[idx], mod.ModuleBase::name(), outputNames[idx]);
  }
}

inline void VcdScope::monitorClock() {
  monitor(_sys.getClockSignalPointer(), 1, "System", "CLK");
}
