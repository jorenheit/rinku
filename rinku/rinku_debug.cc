#include <functional>
#include <map>
#include <optional>
#include <sstream>

#include "linenoise/linenoise.h"

#define RINKU_ENABLE_DEBUGGER
#include "rinku.h"

std::string wrapString(std::string const &str, size_t lineWidth, size_t indent = 0) {
  enum State {
    PARSING_WORD,
    PARSING_WHITESPACE,
  };

  std::string newLine = "\n" + std::string(indent, ' ');
  std::string result(indent, ' ');
  size_t count = indent;
  std::string currentWord;
  State state = PARSING_WHITESPACE;
  
  for (char c: str) {
    switch (state) {
    case PARSING_WHITESPACE: {
      if (c == '\n') {
	result += newLine;
	count = indent;
      }
      else if (std::isspace(c)) {
	if (count + 1 > lineWidth) {
	  result += newLine;
	  count = indent;
	}
	result += c;
	++count;
      }
      else {
	currentWord.clear();
	currentWord += c;
	state = PARSING_WORD;
      }
      break;
    }
    case PARSING_WORD: {
      if (!std::isspace(c)) {
	currentWord += c;
      }
      else {
	if (count + currentWord.length() > lineWidth) {
	  result += newLine;
	  count = indent;
	}
	result += currentWord;

	if (c == '\n') {
	  result += newLine;
	  count = indent;
	}
	else {
	  result += c;
	  count += currentWord.length() + 1;
	}
	state = PARSING_WHITESPACE;
      }
      break;
    }
    }
  }
  
  if (not currentWord.empty()) {
    if (count + currentWord.length() > lineWidth) {
      result += newLine;
    }
    result += currentWord;
  }

  return result;
}

template <typename Int>
bool stringToInt(std::string const &str, Int &result, int base = 10) {
  size_t pos;
  try {
    result = std::stoi(str, &pos, base); }
  catch (...) {
    return false;
  }
  return (pos == str.size());
}

void trim(std::string &str) {
  size_t idx = 0;
  while (idx < str.length() && std::isspace(str[idx])) { ++idx; }
  if (idx == str.length()) {
    str.clear();
    return;
  }
        
  size_t const start = idx;
  idx = str.length() - 1;
  while (idx >= 0 && std::isspace(str[idx])) { --idx; }
  size_t const stop = idx;
        
  str = str.substr(start, stop - start + 1);
}


std::vector<std::string> split(std::string const &str, std::string const &token, bool allowEmpty = false) {
  std::vector<std::string> result;

  size_t prev = 0;
  size_t current = 0;
  while ((current = str.find(token, prev)) != std::string::npos) {
    std::string part = str.substr(prev, current - prev);
    trim(part);
    if (allowEmpty || !part.empty()) result.push_back(part);
    prev = current + token.length();
  }
  std::string last = str.substr(prev);
  trim(last);
  if (allowEmpty || !last.empty()) result.push_back(last);

  return result;
}

std::vector<std::string> split(std::string const &str, char const c, bool allowEmpty = false) {
  return split(str, std::string{c}, allowEmpty);
}

std::string toBinaryString(size_t num, size_t minBits = 0) {
  std::string binary = std::bitset<64>(num).to_string();
  size_t firstOne = binary.find('1');
  std::string trimmed = (firstOne == std::string::npos) ? "0" : binary.substr(firstOne);
    
  if (trimmed.size() < minBits) {
    trimmed = std::string(minBits - trimmed.size(), '0') + trimmed;
  }
    
  return trimmed;
}


template <typename ... Args>
void printMsg(Args&& ... args) {
  (std::cout << ... << std::forward<Args>(args)) << std::endl;
}
      
template <typename ... Args>
void printWarning(Args&& ... args) {
  printMsg("WARNING: ", std::forward<Args>(args)...);
}

template <typename ... Args>
void printError(Args&& ... args) {
  printMsg("ERROR: ", std::forward<Args>(args)...);
}

std::unordered_set<std::string> const *completionCandidates = nullptr;

void completionCallback(char const *buf, linenoiseCompletions *lc) {
  assert(completionCandidates != nullptr);

  static constexpr auto tolower = [](std::string str) -> std::string {
    for (char &c: str) c = std::tolower(c);
    return str;
  };

  std::unordered_set<std::string> const &candidates = *completionCandidates;
  std::vector<std::string> input = split(tolower(buf), ' ');
  
  for (std::string const &cand: candidates) {
    if (input.back() == tolower(cand.substr(0, input.back().length()))) {
      std::string line;
      for (size_t idx = 0; idx != input.size() - 1; ++idx)
	line += (input[idx] + " ");
      line += cand;
      linenoiseAddCompletion(lc, line.c_str());
    }
  }
}

namespace Rinku {
  class Debugger {
    class CommandLine {
    public: 
      using CommandReturn = bool;
      using CommandArgs = std::vector<std::string>;
      using CommandFunction = std::function<CommandReturn (CommandArgs const &)>;
  
    private:
      std::unordered_set<std::string> completionCandidates;
      std::map<std::string, CommandFunction> cmdMap;
      std::map<std::string, std::string> descriptionMap;
      std::map<std::string, std::string> helpMap;
      std::map<std::string, std::vector<std::string>> aliasMap;

      template <typename Callable, typename Ret = std::invoke_result_t<Callable, CommandArgs>>
      void add(std::string const &cmdName, Callable&& fun, std::string const &description, 
	       std::string const &help, std::string const &aliasTarget) {
      
	assert(!cmdMap.contains(cmdName) && "Duplicate command");
      
	cmdMap.insert({cmdName, [=](CommandArgs const &args) -> CommandReturn {
	  if constexpr (std::is_same_v<Ret, void>) {
	    fun(args);
	    return false;
	  }
	  else if constexpr (std::is_same_v<Ret, bool>) {
	    return fun(args);
	  }
	  UNREACHABLE__;
	}});
    
	helpMap.insert({cmdName, help});
	if (cmdName == aliasTarget) {
	  descriptionMap.insert({cmdName, description});
	}
	else {
	  aliasMap[aliasTarget].push_back(cmdName);
	}

	registerCompletionCandidates(cmdName);
      }
  
    public:
      CommandLine() {
	::completionCandidates = &completionCandidates;
      }
      
      template <typename Callable>
      void add(std::initializer_list<std::string> const &aliases, Callable &&fun, 
	       std::string const &description, std::string const &help = "") {
	std::vector aliasVec = aliases;
	for (std::string const &cmdName: aliasVec) {
	  add(cmdName, std::forward<Callable>(fun), description, help, aliasVec[0]);
	}
      }
  
      CommandReturn exec(CommandArgs const &args) {
	assert(args.size() > 0 && "empty arg list");
	if (!cmdMap.contains(args[0])) {
	  printError(args[0], ": Unknown command.");
	  return false;
	}
	return (cmdMap.find(args[0])->second)(args);
      }

      void registerCompletionCandidates(std::string const &str) {
	completionCandidates.insert(str);
      }

      void registerCompletionCandidates(std::vector<std::string> const &vec) {
	for (std::string const &str: vec) {
	  registerCompletionCandidates(str);
	}
      }
      
      void printHelp() {
	std::vector<std::string> commandStrings;
	std::vector<std::string> descriptions;
	size_t maxLength = 0;
	for (auto const &[cmd, description]: descriptionMap) {
	  std::string str = cmd;
	  for (auto const &alias: aliasMap[cmd])
	    str += "|" + alias;
      
	  if (str.length() > maxLength) maxLength = str.length();
	  commandStrings.push_back(str);
	  descriptions.push_back(description);
	}
	assert(commandStrings.size() == descriptions.size());
    
	std::cout << "\nAvailable commands:\n";
	for (size_t idx = 0; idx != commandStrings.size(); ++idx) {
	  std::cout << std::setw(maxLength + 2) << std::setfill(' ') << commandStrings[idx] 
		    << " - " << descriptions[idx] << '\n';
	}
    
	std::cout << "\nType \"help <command>\" for more information about a specific command.\n\n";
      }
  
      void printHelp(std::string const &cmd) {
	if (!helpMap.contains(cmd)) {
	  printError(cmd, ": Unknown command.");
	  return;
	}
    
	std::string const &help = helpMap.find(cmd)->second;
	if (help.empty()) {
	  printMsg(cmd, ": No additional help available.");
	  return;
	}
	std::cout << '\n' << help << "\n\n";
      }
      
    }; // class CommandLine


    enum SignalType {
      Input,
      Output,
      Unknown
    };

    std::string signalTypeString(SignalType s) {
      static std::string const labels[] = {
	"input", "output"
      };
      return labels[s];
    }

    enum NumberFormat {
      Decimal = 10,
      Hexadecimal = 16,
      Binary = 2
    };

    std::string numberFormatString(NumberFormat f) {
      return (f == Decimal) ? "decimal" : ((f == Hexadecimal) ? "hexadecimal" : "binary");
    };
    
    enum Event {
      High,
      Low,
      Rising,
      Falling,
      Change,
      Equals
    };

    static std::string eventString(Event e) {
      static std::string const labels[] = {
	"high", "low", "rising", "falling", "change"
      };
      return labels[e];
    }

    std::string eventString(signal_t value) {
      return "equals " + std::to_string(value) + " (base 10)";
    }
    
    class Breakpoint {
    public:

    private:
      using Getter = std::function<signal_t()>;
      
      Event _trigger;
      std::vector<signal_t> _currentValues;
      std::vector<Getter> _getters;
      std::string _label;
      signal_t _value = 0;

    public:
      Breakpoint(signal_t value, std::string const &label):
	_trigger(Event::Equals),
	_label(label),
	_value(value)
      {}
      
      Breakpoint(Event trigger, std::string const &label):
	_trigger(trigger),
	_label(label)
      {}

      template <typename Callable>
      void add(Callable&& get) {
	_getters.push_back(get);
	_currentValues.push_back(get());
      }
      
      std::string const &label() const {
	return _label;
      }

      bool update() {
	std::vector<signal_t> newValues;
	for (auto const &get: _getters) {
	  newValues.push_back(get());
	}
	bool isTriggered = false;
	for (size_t idx = 0; idx != newValues.size(); ++idx) {
	  if (triggered(_currentValues[idx], newValues[idx])) {
	    isTriggered = true;
	    break;
	  }
	}

	_currentValues.swap(newValues);
	return isTriggered;
      }

    private:
      bool triggered(signal_t oldValue, signal_t newValue) const {
	switch (_trigger) {
	case High: return (newValue != 0);
	case Low: return (newValue == 0);
	case Rising: return (oldValue == 0) && (newValue != 0);
	case Falling: return (oldValue != 0) && (newValue == 0);
	case Change: return (oldValue != newValue);
	case Equals: return (newValue == _value);
	default: UNREACHABLE__;
	}
      }
    };
    
    System &_sys;
    std::vector<Breakpoint> _breakpoints;
    size_t _tick = 0;
    NumberFormat _fmt = NumberFormat::Decimal;
    std::vector<Impl::ModuleBase*> _enableUpdateQueue;;
    static constexpr size_t LINE_WIDTH = 80;
    static constexpr size_t HELP_INDENT = 2;
    
  public:
    Debugger(System &sys):
      _sys(sys)
    {}
    
    void debug() {
      // Construct prompt and helper function (lambda) that wraps linenoise
      std::string const prompt = "rdb> ";
      auto promptAndGetInput = [&prompt]() -> std::pair<std::string, bool> {
	char *line = linenoise(prompt.c_str());
	if (line == nullptr) return {"", false};
            
	linenoiseHistoryAdd(line);
	std::string input(line);
	linenoiseFree(line);
	return {input, true};
      };

      // Create commandline object
      CommandLine cli = generateCommandLine();

      // Setup completion callback
      linenoiseSetCompletionCallback(completionCallback);
      
      // Add completion targets
      for (std::string const &modName: _sys.namedModules()) {
	cli.registerCompletionCandidates(modName);
	auto optModule = tryGetModulePointer(modName);
	assert(optModule.has_value());

	cli.registerCompletionCandidates(optModule.value()->getInputSignalNames());
	cli.registerCompletionCandidates(optModule.value()->getOutputSignalNames());
      }
      std::vector<std::string> const keywords = {
	"in", "out", "bin", "dec", "hex",
	"rising", "falling", "high", "low", "value"
      };

      cli.registerCompletionCandidates(keywords);

      // Start interactive session -> return true/false to indicate if the images should be writen to disk
      std::cout << "<Rinku Debugger> Type \"help\" for a list of available commands.\n\n";
      while (true) {
	auto [input, good] = promptAndGetInput();
	if (!good) return;

	auto args = split(input, ' ');
	if (args.empty()) continue;

	bool quit = cli.exec(args);
	if (quit) return;
      }
      UNREACHABLE__;
    }

  private:
    template <typename ExpectedError, typename Callable, typename Return = std::invoke_result_t<Callable>>
    std::optional<Return> handle_error(Callable&& fun) {
      try {
	return fun();
      }
      catch (ExpectedError const &err) {
	printError(err.what());
      }
      catch (Error::Exception const &err) {
	printError("Unexpected error: " + err.what());
      }
      return {};
    }

    enum ListType {
      Clean,
      Bullets,
      Dashes,
      Numbered
    };
    
    void listVector(ListType listType,
		    std::string const &header,
		    std::vector<std::string> const &vec,
		    std::string const &footer = "") {

      auto const getBullet = [&listType](size_t idx) -> std::string {
	switch (listType) {
	case Clean: return "";
	case Bullets: return "*";
	case Dashes: return "-";
	case Numbered: return (std::to_string(idx) + ".");
	default: UNREACHABLE__;
	};
      };
      
      std::cout << '\n' << header << '\n';
      for (size_t idx = 0; idx != vec.size(); ++idx) {
	std::cout << "  " << getBullet(idx) << " " << vec[idx] << '\n';
      }
      if (vec.size() == 0)
	std::cout << "  -\n";
      
      std::cout << footer << std::endl;
    }

    std::string formatValue(signal_t value) const {
      std::ostringstream oss;
      switch (_fmt) {
      case Decimal: {
	oss << (int)value;
	break;
      }
      case Hexadecimal: {
	oss << "0x" << std::setw(4) << std::setfill('0') << std::hex << value;
	break;
      }
      case Binary: {
	oss << "0b" << toBinaryString(value, 8);
	break;
      }
      }
      return oss.str();
    }

    
    std::string where() const {
      std::ostringstream oss;
      oss << (_tick / 2) << " (" << ((_tick & 1) ? "high" : "low") << ")";
      return oss.str();
    }

    std::optional<Impl::ModuleBase*> tryGetModulePointer(std::string const &modName) {
      return this->template handle_error
	<Error::InvalidModuleName>([&]() -> Impl::ModuleBase* {
	  return &_sys.getModule<Impl::ModuleBase>(modName);
	});
    }

    void flushEnableUpdateQueue() {
      if (_enableUpdateQueue.empty()) return;
      for (Impl::ModuleBase *m: _enableUpdateQueue) {
	m->enableUpdate(true);
      }
      _enableUpdateQueue.clear();
    }
    
    SignalType getSignalType(Impl::ModuleBase const &mod, std::string const &sigName) {
      auto const inputSignalVec = mod.getInputSignalNames();
      auto const outputSignalVec = mod.getOutputSignalNames();

      SignalType sigType = SignalType::Unknown;
      for (std::string const &sig: inputSignalVec) {
	if (sig == sigName) {
	  sigType = SignalType::Input;
	  break;
	}
      }
      if (sigType == Unknown) {
	for (std::string const &sig: outputSignalVec) {
	  if (sig == sigName) {
	    sigType = SignalType::Output;
	    break;
	  }
	}
      }
      return sigType;
    }
    
    
    void listModules() {
      listVector(Bullets, "Available modules:", _sys.namedModules());
    }

    bool listSignals(std::string const &modName, SignalType sigType) {
      auto optModule = tryGetModulePointer(modName);
      if (!optModule.has_value()) return false;
      
      Impl::ModuleBase const *mod = optModule.value();
      listVector(Bullets, signalTypeString(sigType) + " signals for module \"" + modName + "\": ",
		 (sigType == SignalType::Input) ? mod->getInputSignalNames() : mod->getOutputSignalNames());

      return true;
    }
    
    void listSignals(std::string const &modName) {
      if (!listSignals(modName, SignalType::Input)) return;
      listSignals(modName, SignalType::Output);
    }

    void listBreakpoints() {
      std::vector<std::string> brVec;
      for (Breakpoint const &br: _breakpoints) {
	brVec.push_back(br.label());
      }
      listVector(Numbered, "Active breakpoints: ", brVec);
    }

    bool setBreakpointAny(std::string const &modName, SignalType sigType, Event event) {
      auto optModule = tryGetModulePointer(modName);
      if (!optModule.has_value()) return false;
      
      // Make new breakpoint
      std::ostringstream oss;
      oss << modName << ": any " << signalTypeString(sigType) << " -> " << eventString(event);
      std::string newLabel = oss.str();
      for (Breakpoint const &br: _breakpoints) {
	if (br.label() == newLabel) {
	  return true;
	}
      }

      // Add signals to breakpoint and store
      Impl::ModuleBase const *mod = optModule.value();
      auto const signalVec = (sigType == Input) ? mod->getInputSignalNames() : mod->getOutputSignalNames();
      Breakpoint br(event, newLabel);
      for (std::string const &signal: signalVec) {
	br.add([=](){
	  return (sigType == Input) ? mod->getInput(signal) : mod->getOutput(signal);
	});
      }
      _breakpoints.push_back(br);
      return true;
    }

    template <typename EventOrValue>
    bool setBreakpoint(std::string const &modName, std::string const &sigName, EventOrValue trigger) {
      auto optModule = tryGetModulePointer(modName);
      if (!optModule.has_value()) return false;
      
      Impl::ModuleBase const *mod = optModule.value();
      SignalType sigType = getSignalType(*mod, sigName);
      if (sigType == Unknown) {
	printError("Signal \"", sigName, "\" is not available for module \"", modName, "\".\n",
		   "Type \"list ", modName, "\" for a list of available signals.");
	return false;
      }
      
      std::ostringstream oss;
      oss << modName << ": signal \"" << sigName << "\" -> " << eventString(trigger);
      Breakpoint br(trigger, oss.str());
      for (Breakpoint const &current: _breakpoints) {
	if (br.label() == current.label())
	  return true;
      }

      br.add([=](){
	signal_t value = (sigType == Input) ? mod->getInput(sigName) : mod->getOutput(sigName);
	return value;
      });
      _breakpoints.push_back(br);
      return true;
    }

    bool deleteBreakpoint(size_t idx = -1) {
      if (idx == -1UL) {
	_breakpoints.clear();
	return true;
      }
      
      if (idx >= _breakpoints.size()) {
	printError("Breakpoint index out of range.\nType \"break\" to see the list of currently active breakpoints.");
	return false;
      }

      _breakpoints.erase(_breakpoints.begin() + idx);
      return true;
    }

    void run() {
      bool breakpointTriggered = false;
      while (!breakpointTriggered) {
	++_tick;
	bool running = _sys.halfStep();
	flushEnableUpdateQueue();
	if (!running) break;
	    
	for (Breakpoint &br: _breakpoints) {
	  if (br.update())  breakpointTriggered = true;
	}
      }

      if (breakpointTriggered) {
	printMsg("\nBreakpoint triggered @ ", where());
      }
    }

    void step(size_t n) {
      halfStep(2 * n - 1, false);
      halfStep(1);
    }

    void halfStep(size_t n, bool printWhere = true) {
      _tick += n;
      for (size_t i = 0; i != n; ++i) {
	_sys.halfStep();
	flushEnableUpdateQueue();
      }
      if (printWhere) {
	printMsg('\n', where());
      }
    }
    
    void reset() {
      _tick = 0;
      _sys.reset();
      printMsg("System reset.\n", where());
    }

    bool peek(std::string const &modName) {
      if (!peek(modName, Input)) return false;
      return peek(modName, Output);
    }
    
    bool peek(std::string const &modName, SignalType sigType) {
      auto optModule = tryGetModulePointer(modName);
      if (!optModule.has_value()) return false;
      
      Impl::ModuleBase const &mod = *optModule.value();
      std::vector<std::string> signals = (sigType == Input) ? mod.getInputSignalNames() : mod.getOutputSignalNames();

      size_t maxSignalChars = 0;
      for (std::string const &str: signals) {
	maxSignalChars = std::max(maxSignalChars, str.length());
      }

      std::vector<std::string> result;
      for (size_t idx = 0; idx != signals.size(); ++idx){
	signal_t value = (sigType == Input) ? mod.getInput(signals[idx]) : mod.getOutput(signals[idx]);
	
	std::ostringstream line;
	line << std::setw(maxSignalChars) << signals[idx] << ": " << formatValue(value);
	result.push_back(line.str());
      }

      std::string const header = "Module \"" + modName + "\" " + ((sigType == Input) ? "inputs:" : "outputs:");
      listVector(Clean, header, result);
      return true;
    }

    void poke(std::string const &modName, std::string const &sigName, signal_t value) {
      auto optModule = tryGetModulePointer(modName);
      if (!optModule.has_value()) return;

      Impl::ModuleBase &mod = *optModule.value();

      SignalType sigType = getSignalType(mod, sigName);
      if (sigType == Unknown) {
	printError("Signal \"", sigName, "\" is not a valid signal for module \"", modName, "\".");
	return;
      }
      if (sigType == Input) {
	printError("Cannot poke input-signal \"", sigName, "\".");
	return;
      }

      mod.setOutput(sigName, value);
      mod.enableUpdate(false);
      _sys.updateAll();

      // Module will be enabled again after the next clock-cycle
      // in order for the poked signal to have an effect on the
      // rest of the system
      _enableUpdateQueue.push_back(&mod);
    }
    
    CommandLine generateCommandLine() {
      // TODO: add command to view connections? Somehow?
      CommandLine cli;
    
#define COMMAND [&](CommandLine::CommandArgs const &args)
  
      cli.add({"help", "h"}, COMMAND {
	  if (args.size() == 1) {
	    cli.printHelp();
	  }
	  else if (args.size() > 2) {
	    printError(args[0], " command expects at most 1 argument.");
	  }
	  else cli.printHelp(args[1]);
	},
	"Display this text."
	);

      cli.add({"quit", "q"}, COMMAND {
	  return true;
	},
	"Quit the debugger."
	);
      
      cli.add({"list", "l"}, COMMAND {
	  if (args.size() == 1) {
	    listModules();
	  }
	  else if (args.size() == 2) {
	    listSignals(args[1]);
	  }
	  else if (args.size() == 3) {
	    if (args[2] == "in") listSignals(args[1], SignalType::Input);
	    else if (args[2] == "out") listSignals(args[1], SignalType::Output);
	    else {
	      printError(args[0], ": expected 'in' or 'out'.");
	      cli.printHelp(args[0]);
	    }
	  }
	  else {
	    cli.printHelp(args[0]);
	  }
	},
	"List available modules or signals.",
	wrapString("Syntax: list\n"
		   "        list [module]\n"
		   "\n"
		   "Without any arguments, the 'list' command will list all available modules. "
		   "Only labeled modules will be available and can be interacted with. When a "
		   "module label (one from the list) is passed as an argument, the available "
		   "inputs and outputs for this module are listed.",
		   LINE_WIDTH, HELP_INDENT)
	);

      cli.add({"break", "b"}, COMMAND {
	  if (args.size() == 1) {
	    listBreakpoints();
	  }
	  else if (args.size() == 2) {
	    if (!setBreakpointAny(args[1], SignalType::Input, Event::Change)) return;
	    if (!setBreakpointAny(args[1], SignalType::Output, Event::Change)) return;
	  }
	  else if (args.size() == 3) {
	    if (args[2] == "in") setBreakpointAny(args[1], SignalType::Input, Event::Change);
	    else if (args[2] == "out") setBreakpointAny(args[1], SignalType::Output, Event::Change);
	    else {
	      printError(args[0], ": expected 'in' or 'out'.");
	      cli.printHelp(args[0]);
	    }
	  }
	  else if (args.size() == 4) {
	    if (args[3] == "change") setBreakpoint(args[1], args[2], Event::Change);
	    else if (args[3] == "high") setBreakpoint(args[1], args[2], Event::High);
	    else if (args[3] == "low") setBreakpoint(args[1], args[2], Event::Low);
	    else if (args[3] == "rising") setBreakpoint(args[1], args[2], Event::Rising);
	    else if (args[3] == "falling") setBreakpoint(args[1], args[2], Event::Falling);
	    else {
	      printError(args[0], ": unexpected argument '", args[3], "'.");
	      cli.printHelp(args[0]);
	    }
	  }
	  else if (args.size() == 5) {
	    if (args[3] == "value") {
	      signal_t value;
	      if (!stringToInt(args[4], value, _fmt)) {
		printError(args[0], ": value must be an integer in base ", _fmt, ".");
		cli.printHelp(args[0]);
		return;
	      }
	      setBreakpoint(args[1], args[2], value);
	    }
	  }
	  else {
	    printError(args[0], ": too many arguments.");
	    cli.printHelp(args[0]);
	  }
	},
        "Add a breakpoint or view active breakpoints.",
        wrapString("Syntax: break\n"
                   "        break [module]\n"
                   "        break [module] 'in'/'out'\n"
                   "        break [module] [signal] 'high'/'low'/'rising'/'falling'/'change'\n"
                   "        break [module] [signal] 'value' [value]\n"
                   "\n\n"
                   "Without any arguments, 'break' will list the currently active breakpoints."
                   "If only a module label is passed, the breakpoint will be  activated when any of "
                   "the module's inputs or outputs changes state. This can be narrowed down to either "
                   "the inputs or outputs by passing 'in' or 'out' respectively after the module name."
                   "\n\n"
                   "Breakpoints can also be set to specific signal events by passing the signal "
                   "name followed by one of 'high', 'low', 'rising', 'falling' and 'change'."
                   "\n\n"
                   "Finally, a breakpoint can be set to a signal having a specific value, by passing "
                   "'value' followed by a numerical value. This value is interpreted in the base "
                   "currently set by 'format' (see 'help format').",
                   LINE_WIDTH, HELP_INDENT)
	);

      cli.add({"delete", "d"}, COMMAND {
	  if (args.size() != 2) {
	    printError(args[0], " expects exactly 1 argument.");
	    cli.printHelp(args[0]);
	    return;
	  }

	  if (args[1] == "*") {
	    deleteBreakpoint(); // delete all
	    return;
	  }
	  
	  int index;
	  if (!stringToInt(args[1], index) || index < 0) {
	    printError(args[0], " expects the index of the breakpoint you wish to delete.");
	    cli.printHelp(args[0]);
	    return;
	  }
	  deleteBreakpoint(index);
	},
	"Delete a breakpoint.",
	wrapString("Syntax: delete [index]\n"
		   "        delete *\n"
		   "\n"
		   "Use either an index from the list displayed by the 'break' command (to delete a "
		   "specific breakpoint) or '*' to delete all breakpoints.",
		   LINE_WIDTH, HELP_INDENT)
	);
      
      cli.add({"run", "x"}, COMMAND {
	  if (args.size() > 1) {
	    printError(args[0], ": does not expect any arguments.");
	    cli.printHelp(args[0]);
	    return;
	  }
	  run();
	},
	"Run the system.",
	"Runs the system until a breakpoint is hit or the system finishes (exit or error)."
	);

      cli.add({"step", "S"}, COMMAND {
	  if (args.size() > 2) {
	    printError(args[0], ": does expects at most 1 argument.");
	    cli.printHelp(args[0]);
	    return;
	  }

	  int n = 1;
	  if (args.size() == 2) {
	    if (!stringToInt(args[1], n) || n <= 0) {
	      printError(args[0], ": argument must be a positive integer.");
	      cli.printHelp(args[0]);
	      return;
	    }
	  }
	  
	  step(n);
	},
	"Advance the system by a full cycle."
	);

      cli.add({"half", "s"}, COMMAND {
	  if (args.size() > 2) {
	    printError(args[0], ": does expects at most 1 argument.");
	    cli.printHelp(args[0]);
	    return;
	  }

	  int n = 1;
	  if (args.size() == 2) {
	    if (!stringToInt(args[1], n) || n <= 0) {
	      printError(args[0], ": argument must be a positive integer.");
	      cli.printHelp(args[0]);
	      return;
	    }
	  }
	  
	  halfStep(n);
	},
	"Advance the system by a half cycle."
	);
      
      cli.add({"reset", "r"}, COMMAND {
	  reset();
	  if (args.size() > 1) {
	    printError(args[0], ": does not expect any arguments.");
	    cli.printHelp(args[0]);
	    return;
	  }
	},
	"Reset the system."
	);

      cli.add({"format", "f"}, COMMAND {
	  if (args.size() == 1) {
	    printMsg("Signal values are displayed in ", numberFormatString(_fmt), " format.");
	    return;
	  }
	  else if (args.size() > 2) {
	    printError(args[0], ": expects at most 1 argument.");
	    cli.printHelp(args[0]);
	    return;
	  }

	  if (args[1] == "hex") _fmt = NumberFormat::Hexadecimal;
	  else if (args[1] == "bin") _fmt = NumberFormat::Binary;
	  else if (args[1] == "dec") _fmt = NumberFormat::Decimal;
	  else {
	    printError(args[0], " argument must either be 'dec', 'hex' or 'bin'.");
	    cli.printHelp(args[0]);
	  }
	},
	"Set the signal number-format.",
	wrapString("  Syntax: format\n"
		   "          format 'bin/dec/hex'\n"
		   "\n"
		   "By default, signal values will be displayed in decimal format. This can be changed "
		   "to binary or hexadecimal using this command. Note that When poking values or setting "
		   "breakpoints to specific signal-values, user-input will be interpreted according to "
		   "this setting.",
		   LINE_WIDTH, HELP_INDENT)
	);
      
      cli.add({"peek", "p"}, COMMAND {
	  if (args.size() < 2) {
	    printError(args[0], ": expects at least 1 argument.");
	    cli.printHelp(args[0]);
	  }
	  else if (args.size() == 2) peek(args[1]);
	  else if (args.size() == 3) {
	    if (args[2] == "in") peek(args[1], SignalType::Input);
	    else if (args[2] == "out") peek(args[1], SignalType::Output);
	    else {
	      printError(args[0], ": expected 'in' or 'out'.");
	      cli.printHelp(args[0]);
	    }
	  }
	  else {
	    printError(args[0], ": too many arguments.");
	    cli.printHelp(args[0]);
	  }
	},
	"Peek inside a module.",
	wrapString("Syntax: peek [module]\n"
		   "        peek [module] 'in'/'out'\n"
		   "\n"
		   "When only a module name is passed, all input and output values for this module "
		   "will be listed. By passing either 'in' or 'out' as its second argument, only "
		   "inputs or outputs respectively will be displayed.",
		   LINE_WIDTH, HELP_INDENT)
	);

      cli.add({"poke", "P"}, COMMAND {
	  if (args.size() != 4) {
	    printError(args[0], ": expected exactly 3 arguments.");
	    cli.printHelp(args[0]);
	    return;
	  }

	  int value;
	  if (!stringToInt(args[3], value, _fmt)) {
	    printError(args[0], ": 3rd argument must be a valid integer value.");
	    cli.printHelp(args[0]);
	  }
	  
	  poke(args[1], args[2], value);
	},
	"Change the output of a module.",
	wrapString(
		   "Change the output of a module.\n"
		   "  Syntax: poke [module] [signal] [value]\n"
		   "\n"
		   "When the system is paused because the HLT signal was active or a breakpoint "
		   "was hit (or even before it was started), the outputs of a module can be changed "
		   "using the 'poke' command. After poking a module, all other modules will be "
		   "updated in order to propagate the signal. The module itself will not be updated "
		   "until after the following clock state-change, when the alternative output has "
		   "had its effect.",
		   LINE_WIDTH, HELP_INDENT)
	);
      
      
      
#undef COMMAND
    
      return cli;
    }
  }; // class Debugger
} // namespace Rinku

void Rinku::System::debug() {
  Rinku::Debugger(*this).debug();
}

