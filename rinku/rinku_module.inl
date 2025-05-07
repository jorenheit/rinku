
template <typename T1, typename T2>
void Module<T1, T2>::addOutgoing(size_t outputIndex, int idx) {
  for (int moduleIndex: outputModules[outputIndex])
    if (moduleIndex == idx) return;

  outputModules[outputIndex].push_back(idx);
}

template <typename T1, typename T2>
bool Module<T1, T2>::connected(size_t inputIndex, signal_t const *ptr) {
  for (auto const &pr: inputState[inputIndex])
    if (pr.first == ptr) 
      return true;
  return false;
}


template <typename T1, typename T2>
std::vector<int> const &Module<T1, T2>::outgoing(size_t outputIndex) {
  return outputModules[outputIndex];
}

template <typename T1, typename T2>
std::vector<int> Module<T1, T2>::updateAndCheck() {
  if (guaranteed() || not updateEnabled()) return {};
      
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

template <typename T1, typename T2>
Module<T1, T2>::Module() {
  for (size_t idx = 0; idx != Inputs::N; ++idx) {
    nameToInput.emplace(std::string(Inputs::names()[idx]), idx);
  }

  for (size_t idx = 0; idx != Outputs::N; ++idx) {
    nameToOutput.emplace(std::string(Outputs::names()[idx]), idx);
  }
}

template <typename T1, typename T2>
template <typename InputSignal, typename OutputSignal, typename OtherModule>
void Module<T1, T2>::connect(OtherModule &other) {
  static_assert(std::is_base_of_v<Impl::ModuleBase, OtherModule>,
		"Other module is not a valid Module.");
  assert(getModuleIndex() != -1 && "input module index not set");
  assert(other.getModuleIndex() != -1 && "output module index not set");


  Error::throw_runtime_error_if
    <Error::SystemLocked>(locked(), this->name(), InputSignal::Name, other.ModuleBase::name(), OutputSignal::Name);
			    
      
  constexpr size_t inputIndex = index_of<InputSignal>;
  constexpr size_t outputIndex = OtherModule::template index_of<typename OutputSignal::Base>;

  signal_t *ptr = &other.outputState[outputIndex];
  if (connected(inputIndex, ptr)) return;
    
  other.addOutgoing(outputIndex, getModuleIndex());
  inputState[inputIndex].push_back({ptr, OutputSignal::ActiveLow});
  addDotConnection<InputSignal, OutputSignal>(other);
}

template <typename T1, typename T2>
template <typename InputSignal, signal_t Value>
void Module<T1, T2>::connect() {
  static signal_t const value = Value;

  static_assert(Value < (1 << InputSignal::Width),
		"Constant input value too large for signal-width.");

  Error::throw_runtime_error_if
    <Error::SystemLocked>(locked(), ModuleBase::name(), InputSignal::Name, "CONST(" + std::to_string(value) + ")", "OUT"); 

  signal_t const *ptr = &value;
  constexpr size_t inputIndex = index_of<InputSignal>;

  if (connected(inputIndex, ptr)) return;
  inputState[inputIndex].push_back({ptr, false});
  addDotConnection<InputSignal, Value>();
  ModuleBase::addHardwiredValue(Value);
}

template <typename T1, typename T2>
template <typename S>
void Module<T1, T2>::setOutput(signal_t value) {
  static_assert(S::IsOutput,
		"Signal passed to setOutput is not an output signal.");
      
  setOutput(index_of<S>, value);
}

template <typename T1, typename T2>
void Module<T1, T2>::setOutput(std::string const &signalName, signal_t value) {
  Error::throw_runtime_error_if
    <Error::InvalidSignalName>(!nameToOutput.contains(signalName), ModuleBase::name(), signalName);

  return setOutput(nameToOutput[signalName], value);
}

template <typename T1, typename T2>
void Module<T1, T2>::setOutput(size_t outputIndex, signal_t value) {
  Error::throw_runtime_error_if
    <Error::OutputChangeNotAllowed>(!setOutputAllowed(), ModuleBase::name());

  Error::throw_runtime_error_if
    <Error::IndexOutOfBounds>(outputIndex >= Outputs::N, "output", ModuleBase::name(), outputIndex, Outputs::N);

  outputState[outputIndex] = value & Outputs::masks()[outputIndex];
}

template <typename T1, typename T2>
template <typename S>
signal_t Module<T1, T2>::getOutput() const {
  static_assert(S::IsOutput,
		"Signal passed to getOutput is not an output-signal.");

  return getOutput(index_of<S>);
}


template <typename T1, typename T2>
template <typename S>
signal_t Module<T1, T2>::getInput() const {
  static_assert(S::IsInput,
		"Signal passed to getInput is not an input-signal.");
      
  return getInput(index_of<S>);
}

template <typename T1, typename T2>
signal_t Module<T1, T2>::getInput(std::string const &signalName) const {
  Error::throw_runtime_error_if
    <Error::InvalidSignalName>(!nameToInput.contains(signalName), ModuleBase::name(), signalName);
      
  return getInput(nameToInput.find(signalName)->second);
}

template <typename T1, typename T2>
signal_t Module<T1, T2>::getInput(size_t inputIndex) const {
  Error::throw_runtime_error_if
    <Error::IndexOutOfBounds>(inputIndex >= Inputs::N, "input", ModuleBase::name(), inputIndex, Inputs::N);
      
  signal_t result = 0;
  for (auto const &[ptr, activeLow]: inputState[inputIndex]) {
    if (ptr) result |= (activeLow ? ~(*ptr) : *ptr);
  }
  return result & Inputs::masks()[inputIndex];
}

template <typename T1, typename T2>
signal_t Module<T1, T2>::getOutput(std::string const &signalName) const {
  Error::throw_runtime_error_if
    <Error::InvalidSignalName>(!nameToOutput.contains(signalName), ModuleBase::name(), signalName);

  return getOutput(nameToOutput.find(signalName)->second);
}

template <typename T1, typename T2>
signal_t Module<T1, T2>::getOutput(size_t outputIndex) const {
  Error::throw_runtime_error_if
    <Error::IndexOutOfBounds>(outputIndex >= Outputs::N, "output", ModuleBase::name(), outputIndex, Outputs::N);
      
  return outputState[outputIndex] & Outputs::masks()[outputIndex];
}

template <typename T1, typename T2>
template <typename InputSignal, typename OutputSignal, typename OtherModule>
void Module<T1, T2>::addDotConnection(OtherModule const &otherMod) {
  std::ostringstream oss;

  oss << otherMod.ModuleBase::name()    << ":" << OutputSignal::Name << " -> "
      << ModuleBase::name() << ":" << InputSignal::Name;

  if (OutputSignal::ActiveLow) {
    oss << " [style=dashed]";
  }
  
  ModuleBase::addDotConnection(oss.str());
}

template <typename T1, typename T2>
template <typename InputSignal, signal_t Value>
void Module<T1, T2>::addDotConnection() {
  std::ostringstream oss;
  oss << Value << " -> " << name() << ":" << InputSignal::Name;
  ModuleBase::addDotConnection(oss.str());
}
