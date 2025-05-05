#ifndef RINKU_DEBUG_BREAKPOINTS_H
#define RINKU_DEBUG_BREAKPOINTS_H

#include "rinku.h"
#include <functional>
#include <vector>
#include <memory>

namespace Rinku {

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

  
  class BreakpointBase {
    std::string _label;
  public:
    BreakpointBase(std::string const &label):
      _label(label)
    {}

    std::string const &label() const {
      return _label;
    }

    virtual bool update() = 0;
      
  }; // class BreakpointBase



  
  class Breakpoint: public BreakpointBase {
  public:

  private:
    using Getter = std::function<signal_t()>;
      
    Event _trigger;
    std::vector<signal_t> _currentValues;
    std::vector<Getter> _getters;
    signal_t _value = 0;

  public:
    Breakpoint(signal_t value, std::string const &label):
      BreakpointBase(label),
      _trigger(Event::Equals),
      _value(value)
    {}
      
    Breakpoint(Event trigger, std::string const &label):
      BreakpointBase(label),
      _trigger(trigger)
    {}

    template <typename Callable>
    void add(Callable&& get) {
      _getters.push_back(get);
      _currentValues.push_back(get());
    }
      
    virtual bool update() override {
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
  }; // class Breakpoint



  
  class CombinedBreakpoint: public BreakpointBase {
  public:
    enum Operation {
      And,
      Nand,
      Or,
      Nor,
      Xor,
      Xnor,
      N_OPERATIONS
    };

  private:
    std::shared_ptr<BreakpointBase> _br1;
    std::shared_ptr<BreakpointBase> _br2;
    Operation _op;
      
    static constexpr std::string operationStrings[N_OPERATIONS] = {
      "and", "nand", "or", "nor", "xor"
    };

    std::string constructLabel(std::string const &br1Label, std::string const &br2Label, Operation op) {
      auto const applyIndentation = [](std::string const &str) -> std::string {
	std::string result = "  ";
	for (char c: str) {
	  if (c != '\n') result += c;
	  else result += "\n  ";
	}
	return result;
      };
	
      std::string result = "Combined breakpoint: " + operationStrings[op] + "\n";
      result += applyIndentation(br1Label) + "\n";
      result += applyIndentation(br2Label);
      return result;
    }
      
  public:
    CombinedBreakpoint(std::shared_ptr<BreakpointBase> br1,
		       std::shared_ptr<BreakpointBase> br2,
		       Operation op):
      BreakpointBase(constructLabel(br1->label(), br2->label(), op)),
      _br1(br1),
      _br2(br2),
      _op(op)
    {}

    static std::string operationToString(Operation op) {
      return operationStrings[op];
    }

    static Operation stringToOperation(std::string const &str) {
      for (size_t idx = 0; idx != N_OPERATIONS; ++idx) {
	if (str == operationStrings[idx]) {
	  return static_cast<Operation>(idx);
	}
      }
      return N_OPERATIONS;
    }
      
    virtual bool update() override {
      bool br1Triggered = _br1->update();
      bool br2Triggered = _br2->update();
      switch (_op) {
      case And:  return br1Triggered && br2Triggered;
      case Nand: return !(br1Triggered && br2Triggered);
      case Or:   return br1Triggered || br2Triggered;
      case Nor:  return !(br1Triggered || br2Triggered);
      case Xor:  return br1Triggered ^ br2Triggered;
      case Xnor: return !(br1Triggered ^ br2Triggered);
      default: UNREACHABLE__;
      }
    }
  }; // class CombinedBreakpoint

} //namespace Rinku
#endif // RINKU_DEBUG_BREAKPOINTS_H
