#ifndef RINKU_MODULEBASE_H
#define RINKU_MODULEBASE_H

namespace Impl {
  class ModuleBase {
    friend class Debugger;
      
    bool _locked = false;
    bool _setOutputAllowed = true;
    bool _guaranteed = false;
    bool _updateEnabled = true;
    int _index = -1;
    std::string _name = "[UNNAMED]";
      
  public:
    class GuaranteeToken {
      friend class ModuleBase;
      bool *value;
      GuaranteeToken(bool *val):
	value(val)
      {}
    public:
      void set() {
	assert(value && "value can't be null");
	*value = true;
      }
    };
      
    virtual ~ModuleBase() = default;
    virtual void clockRising() {}
    virtual void clockFalling() {}
    virtual void update(GuaranteeToken) {}
    virtual void reset() {}
    virtual std::vector<int> updateAndCheck() = 0;
    virtual size_t nInputs() const = 0;
    virtual size_t nOutputs() const = 0;
    virtual signal_t getInput(std::string const &) const = 0;
    virtual signal_t getInput(size_t) const = 0;
    virtual signal_t getOutput(std::string const &) const = 0;
    virtual signal_t getOutput(size_t) const = 0;
    virtual void setOutput(std::string const &, signal_t) = 0;
    virtual void setOutput(size_t, signal_t) = 0;
    virtual std::vector<std::string> getInputSignalNames() const = 0;
    virtual std::vector<std::string> getOutputSignalNames() const = 0;

    void update() {
      this->update(GuaranteeToken{&_guaranteed});
    }

    void setModuleIndex(int idx) {
      _index = idx;
    }

    int getModuleIndex() const {
      return _index;
    }

    void setName(std::string const &name) {
      _name = name;
    }

    std::string const &name() const {
      return _name;
    }

    void enableUpdate(bool val) {
      _updateEnabled = val;
    }
      
    void lock() {
      _locked = true;
    }

    bool locked() const {
      return _locked;
    }

    void allowSetOutput(bool value) {
      _setOutputAllowed = value;
    }

    bool setOutputAllowed() const {
      return _setOutputAllowed;
    }

    void resetGuaranteed() {
      _guaranteed = false;
    }

    bool guaranteed() const {
      return _guaranteed;
    }

    bool updateEnabled() const {
      return _updateEnabled;
    }
  }; // class ModuleBase
    
} // namespace Impl

#endif // RINKU_MODULEBASE_H
