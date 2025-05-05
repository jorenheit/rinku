#include <iostream>
#include <bitset>
#include "rinku_debug_utilities.h"

void listVector(ListType listType, std::string const &header, std::vector<std::string> const &vec, std::string const &footer) {

  auto const getBullet = [&listType](size_t idx) -> std::string {
    switch (listType) {
    case Clean: return "";
    case Bullets: return "*";
    case Dashes: return "-";
    case Numbered: return (std::to_string(idx) + ".");
    default: UNREACHABLE__;
    };
  };

  auto const applyIndentation = [](std::string const &str, size_t n) -> std::string {
    std::string result;
    for (char c: str) {
      if (c != '\n') result += c;
      else result += "\n" + std::string(n, ' ');
    }
    return result;
  };
      
  std::cout << '\n' << header << '\n';
  for (size_t idx = 0; idx != vec.size(); ++idx) {
    std::string pre = "  " + getBullet(idx) + " ";
    std::cout << pre << applyIndentation(vec[idx], pre.size()) << '\n';
  }
  if (vec.size() == 0)
    std::cout << "  -\n";
      
  std::cout << footer << std::endl;
}

std::string wrapString(std::string const &str, size_t lineWidth, size_t indent) {
  
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


std::vector<std::string> split(std::string const &str, std::string const &token, bool allowEmpty) {
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

std::vector<std::string> split(std::string const &str, char const c, bool allowEmpty) {
  return split(str, std::string{c}, allowEmpty);
}

std::string toBinaryString(size_t num, size_t minBits) {
  std::string binary = std::bitset<64>(num).to_string();
  size_t firstOne = binary.find('1');
  std::string trimmed = (firstOne == std::string::npos) ? "0" : binary.substr(firstOne);
    
  if (trimmed.size() < minBits) {
    trimmed = std::string(minBits - trimmed.size(), '0') + trimmed;
  }
    
  return trimmed;
}
