#ifndef SIMPSHELL_H
#define SIMPSHELL_H

#include <functional>
#include <map>
#include <vector>
#include <unordered_set>
#include <cassert>
#include "linenoise/linenoise.h"

#define UNREACHABLE__ assert(false && "UNREACHABLE");

class SimpShell {
 public: 
  using CommandArgs = std::vector<std::string>;
  using CommandFunction = std::function<bool (CommandArgs const &)>;
  
 private:
  std::unordered_set<std::string> completionCandidates;
  std::map<std::string, CommandFunction> cmdMap;
  std::map<std::string, std::string> descriptionMap;
  std::map<std::string, std::string> helpMap;
  std::map<std::string, std::vector<std::string>> aliasMap;

  template <typename Callable, typename Ret = std::invoke_result_t<Callable, CommandArgs>>
  void add(std::string const &cmdName, Callable&& fun, std::string const &description, 
	   std::string const &help, std::string const &aliasTarget);  
 public:
  SimpShell();

  template <typename Callable>
  void add(std::initializer_list<std::string> const &aliases, Callable &&fun, 
	   std::string const &description, std::string const &help = "");

  bool promptAndExecute(std::string const &promptStr);      
  void registerCompletionCandidates(std::string const &str);
  void registerCompletionCandidates(std::vector<std::string> const &vec);      
  void printHelp();  
  void printHelp(std::string const &cmd);

private:
  bool exec(CommandArgs const &args);  
}; // class SimpShell


// Template implementations

template <typename Callable, typename Ret>
void SimpShell::add(std::string const &cmdName, Callable&& fun, std::string const &description, 
		    std::string const &help, std::string const &aliasTarget) {
      
  assert(!cmdMap.contains(cmdName) && "Duplicate command");
      
  cmdMap.insert({cmdName, [=](CommandArgs const &args) -> bool {
    if constexpr (std::is_same_v<Ret, void>) {
      fun(args);
      return true;
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


template <typename Callable>
void SimpShell::add(std::initializer_list<std::string> const &aliases, Callable &&fun, 
		    std::string const &description, std::string const &help) {
  std::vector aliasVec = aliases;
  for (std::string const &cmdName: aliasVec) {
    add(cmdName, std::forward<Callable>(fun), description, help, aliasVec[0]);
  }
}

#endif // SIMPSHELL_H
