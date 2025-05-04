
inline void ModuleBase::update() {
  this->update(GuaranteeToken{&_guaranteed});
}

inline void ModuleBase::setModuleIndex(int idx) {
  _index = idx;
}

inline int ModuleBase::getModuleIndex() const {
      
  return _index;
}

inline void ModuleBase::setName(std::string const &name) {
  _name = name;
}

inline std::string const &ModuleBase::name() const {
  return _name;
}

inline void ModuleBase::enableUpdate(bool val) {
  _updateEnabled = val;
}
      
inline void ModuleBase::lock() {
  _locked = true;
}

inline bool ModuleBase::locked() const {
  return _locked;
}

inline void ModuleBase::allowSetOutput(bool value) {
  _setOutputAllowed = value;
}

inline bool ModuleBase::setOutputAllowed() const {
  return _setOutputAllowed;
}

inline void ModuleBase::resetGuaranteed() {
  _guaranteed = false;
}

inline bool ModuleBase::guaranteed() const {
  return _guaranteed;
}

inline bool ModuleBase::updateEnabled() const {
  return _updateEnabled;
}
