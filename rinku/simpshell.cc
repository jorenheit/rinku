#include <iostream>
#include <iomanip>
#include "simpshell.h"
#include "rinku_debug_utilities.h"

std::unordered_set<std::string> const *completionCandidates = nullptr;

void completionCallback(char const *buf, linenoiseCompletions *lc) {
  assert(completionCandidates != nullptr);

  static constexpr auto tolower = [](std::string str) -> std::string {
    for (char &c: str) c = std::tolower(c);
    return str;
  };

  std::unordered_set<std::string> const &candidates = *completionCandidates;
  std::vector<std::string> input = split(buf, ' ');
  
  for (std::string const &cand: candidates) {
    if (tolower(input.back()) == tolower(cand.substr(0, input.back().length()))) {
      std::string line;
      for (size_t idx = 0; idx != input.size() - 1; ++idx)
	line += (input[idx] + " ");
      line += cand;
      linenoiseAddCompletion(lc, line.c_str());
    }
  }
}

SimpShell::SimpShell() {
  ::completionCandidates = &completionCandidates;
  linenoiseSetCompletionCallback(completionCallback);
}

bool SimpShell::promptAndExecute(std::string const &promptStr) {
  char *line = linenoise((promptStr + " ").c_str());
  if (line == nullptr) return false;
        
  linenoiseHistoryAdd(line);
  auto args = split(line, ' ');
  linenoiseFree(line);
  
  return args.empty() ? true : exec(args);
}
      
bool SimpShell::exec(CommandArgs const &args) {
  assert(args.size() > 0 && "empty arg list");
  if (!cmdMap.contains(args[0])) {
    printError(args[0], ": Unknown command.");
    return true;
  }
  return (cmdMap.find(args[0])->second)(args);
}

void SimpShell::registerCompletionCandidates(std::string const &str) {
  completionCandidates.insert(str);
}

void SimpShell::registerCompletionCandidates(std::vector<std::string> const &vec) {
  for (std::string const &str: vec) {
    registerCompletionCandidates(str);
  }
}
      
void SimpShell::printHelp() {
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
  
void SimpShell::printHelp(std::string const &cmd) {
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
