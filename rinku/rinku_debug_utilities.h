#ifndef RINKU_DEBUG_UTILITIES_H
#define RINKU_DEBUG_UTILITIES_H

#include <vector>
#include <string>
#include <cassert>

#ifndef UNREACHABLE__
#define UNREACHABLE__ assert(false && "UNREACHABLE");
#endif

enum ListType {
  Clean,
  Bullets,
  Dashes,
  Numbered
};
    
void                     listVector(ListType listType, std::string const &header, std::vector<std::string> const &vec, std::string const &footer = "");
std::string              wrapString(std::string const &str, size_t lineWidth, size_t indent = 0);
void                     trim(std::string &str);
std::vector<std::string> split(std::string const &str, std::string const &token, bool allowEmpty = false);
std::vector<std::string> split(std::string const &str, char const c, bool allowEmpty = false);
std::string              toBinaryString(size_t num, size_t minBits = 0);


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


#endif // RINKU_DEBUG_UTILITIES_H
