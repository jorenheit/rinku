# Rinku

<p align="center"><img src="logo.png" alt="Rinku logo" width="200"/></p>


## Introduction

Rinku is a lightweight C++ header-only library designed to simplify the creation of cycle-accurate, event-driven computational systems. By defining *signals* and *modules* that communicate via well-defined interfaces, Rinku enables you to compose complex hardware-like architectures entirely in C++, with automatic propagation of signal changes and clock-driven update semantics.

### Motivation
The library was developed for aid in prototyping and debugging a custom designed breadboard computer, inspired by Ben Eater's 8-bit computer. The resulting system has been added as one of the examples (bfcpu) and has proven to be a valuable tool in the development of such systems. More information about the bfcpu-project can be found on [Github](https://github.com/jorenheit/bfcpu). To incorporate the microcode for the BFCPU into the simulated system, [Mugen](https://github.com/jorenheit/mugen) was used to generate the `microcode.h` and `microcode.cc` source files.

## Installation
Follow these steps to get Rinku up and running on your machine:

1. **Clone the repository**

   ```sh
   git clone https://github.com/jorenheit/rinku.git
   cd rinku
   ```

2. **Install headers system‑wide**

   By default the headers will be installed under `/usr/local/include/rinku`:

   ```sh
   # Install to the default prefix (/usr/local)
   make install
   ```

3. **Verify the installation**

   After installation, you should be able to include Rinku headers in your projects as:

   ```cpp
   #include <rinku/rinku.h>
   #include <rinku/util/bus.h>
   ```

4. **Build the examples**

   We provide a couple of  examples in the `examples/` folder. These examples use relative paths for including the Rinku headers, so you don't have to install the headers to your systems include directory to compile and run them.

   - **Hello‑World**
     ```sh
     cd examples/hello-world
     make         # produces the `hello` executable
     ./hello      # run the example
     ```

   - **BFCPU Emulator**
     ```sh
     cd examples/bfcpu
     make                         # produces the `bfcpu` executable
     ./bfcpu programs/hello.bin   # run `hello world` on the emulator
     ```

Once installed, you can add Rinku to your own projects simply by including the headers and compiling with C++20.


## Workflow

A typical Rinku-based simulation involves these steps:

1. **Define module types** with their input and output signals (using `INPUT`, `OUTPUT`, etc.).
2. **Add modules** to a `Rinku::System`.
3. **Connect** inputs to outputs (or constants) across modules to form your signal topology.
4. **Initialize** the system by calling `system.init()` to lock the configuration and reset internal state.
5. **Run** the simulation cycle-by-cycle via `system.run()`, optionally handling halts and resumes.

## Use of Macro's and Namespaces
Because Rinku is a template-heavy library which might scare some less experienced C++ programmers, macro's are available to provide a consistent and easy-to-use syntax. On top of that, some macro's pass additional information to the underlying functions which allows for better error messages (including the problematic file and line-number). All macro's start with the `RINKU_` prefix, unless the preprocessor constant `RINKU_REMOVE_MACRO_PREFIX` has been defined prior to including the main Rinku header file.

  ```cpp
  #define RINKU_REMOVE_MACRO_PREFIX
  #include <rinku/rinku.h>
  
  // RINKU_INPUT() is now available as INPUT()
  ```
Furthermore, all types defined by Rinku are in the `Rinku` namespace. Simply use a `using namespace Rinku` directive to make them available to the namespace you're working from. The examples in this guide will assume that the macro has been defined and the Rinku namespace is imported.

## Creating a Module
To create a new module to be part of a system we take the following steps:
1. Define input and output signals for the module.
2. Define the module by deriving a new class from the Rinku::Module base-class.
3. Define the module behavior by implementing some functions.

### Step 1: Define Signals

Rinku represents each signal as a strongly-typed struct, carrying a fixed bit-width. Macro's have been provided to simplify the declarations of new signals. 

  ```cpp
  INPUT(MyInput, 8); // MyInput is now available as an 8-bit input-signal
  OUTPUT(MyOutput, 16); // MyOutput is now available as a 16-bit output-signal
  ```

### Step 2: Define Module-Class
To define a new module type as a class, the input and output signals have to be combined in a `SIGNAL_LIST` so they can be passed to the `Rinku::Module<>` base-class. The `SIGNAL_LIST` macro takes the list-name as its first argument, followed by all the signals (previously declared using `INPUT` and `OUTPUT`) that need to be part of this list. A signal-list should contain only 1 type of signal, either inputs or outputs but not both.

  ```cpp
  SIGNAL_LIST(ModuleInputSignals, In1, In2, In3);
  SIGNAL_LIST(ModuleOutputSignals, Out1, Out2, Out3);
  ```
After defining the lists, we can declare our new module, where the `MODULE` macro can be used to handle the derivation details.:

  ```cpp
  class MyModule: MODULE(ModuleInputSignals, ModuleOutputSignals) {
	  // ...
  };
  ```
If you prefer to use normal C++ inheritance syntax, do not forget to derive publicly from the `Rinku::Module`:

  ```cpp
  class MyModule: public Rinku::Module<ModuleInputSignals, ModuleOutputSignals> 
  {
      // ...
  };
  ```
### Step 3: Module Behavior
To define the module behavior, virtual functions from the Module baseclass may be overridden. Macro's have been provided to handle the C++ syntax for you, but of course you are free to use regular C++ as well (optionally using the `virtual` and `override` keywords):

1. **`void clockRising()`/`ON_CLOCK_RISING()`** and **`void clockFalling()`/`ON_CLOCK_FALLING()`** <br/> 
These functions are called every time the clock changes state. Because the order in which the modules are clocked is undefined, the implementation of these functions should not affect its outputs, which in turn may affect other modules depending on the order in which the modules are clocked. Datamembers should therefore be used to keep track of its internal state, which are only made available on the output signals when `update()` is called (see below).
2. **`void update()`** or **`UPDATE()`** <br/>
This function is called in-between clock state-changes and should handle signal propagation within the module. The system will repeatedly call the update-function on all modules until all module-outputs have settled into a stable state. In general, this is the place where the outputs of the module are set, while the clock-functions above are used to change the internal state without affecting the outputs (which could affect other modules).
3. **`void reset()`** or **`RESET()`** <br/>
This function is called on initialization or when `System::reset()` is used to reset the entire system. It should make sure that the module resets to some known predefined state.

Inside these functions, inputs and outputs can be read using functionality from the `Rinku::Module` base-class. Each of these functions has a macro-substitute for better error-messages and simpler syntax.

| C++                        | Macro                       | Description                                                   |
|----------------------------|-----------------------------|---------------------------------------------------------------|
| `getInput<Signal>()`       | `GET_INPUT(Signal)`         | Returns signal-value on the given input (0 if not connected). |
| `getOutput<Signal>()`      | `GET_OUTPUT(Signal)`        | Returns the value currently on the given output.              |
| `setOutput<Signal>(value)` | `SET_OUTPUT(Signal, value)` | Sets the given output to `value`.                             |

#### `signal_t`
The output-signals are stored internally as 64-bit integers (`uint64_t`). The type `Rinku::signal_t` has been provided as a type alias for convenience; this is the return-type for `getInput` and the expected type for `value` when setting outputs.

### Examples
#### Example 1: AND Gate
  
  ```cpp
  INPUT(AND_IN_A, 1);
  INPUT(AND_IN_B, 1);
  OUTPUT(AND_OUT, 1);
  
  SIGNAL_LIST(AndInputs, AND_IN_A, AND_IN_B);
  SIGNAL_LIST(AndOutput, AND_OUT);
  
  class And: MODULE(AndInputs, AndOutput) {
  public:
	UPDATE() {
	  bool a = GET_INPUT(AND_IN_A);
	  bool b = GET_INPUT(AND_IN_B);
	  SET_OUTPUT(AND_OUT, a && b);
	}
  };
  ```
#### Example 2: 8-Bit Register

  ```cpp
  INPUT(REG_DATA_IN, 8);
  INPUT(REG_LD, 1);
  INPUT(REG_EN, 1);
  OUTPUT(REG_DATA_OUT, 8);
  
  SIGNAL_LIST(RegisterInputs, REG_DATA_IN, REG_LD, REG_EN);
  SIGNAL_LIST(RegisterOutputs, REG_DATA_OUT);
  
  class Register: MODULE(RegisterInputs, RegisterOutputs) {
	unsigned char const resetValue;
    unsigned char value;
  public:
	Register(unsigned char val = 0):
	  resetValue(val),
	  value(val)
	{}
  
    ON_CLOCK_RISING() {
	  if (GET_INPUT(REG_LD)) {
	    value = GET_INPUT(REG_DATA_IN);
	  }
	}
	
	UPDATE() {
	  SET_OUTPUT(REG_DATA_OUT, GET_INPUT(REG_EN) ? value : 0);
	}
	
	RESET() {
	  value = 0;
	}
  };
  ```
### Predefined Modules
A few convenient modules that are ready for use can be included from the `rinku/logic` and `rinku/util` folders. Below is an overview of these modules and their signals.

| Module        | Namespace      | Inputs (Bits)                             | Outputs (Bits)                              | Header                    |
|---------------|----------------|-------------------------------------------|---------------------------------------------|---------------------------|
| `And`         | `Rinku::Logic` | `AND_IN_A (1)`, `AND_IN_B (1)`            | `AND_OUT (1)`                               | `<rinku/logic/logic.h>`   |
| `Or`          | `Rinku::Logic` | `OR_IN_A (1)`, `OR_IN_B (1)`              | `OR_OUT (1)`                                | `<rinku/logic/logic.h>`   |
| `Xor`         | `Rinku::Logic` | `XOR_IN_A (1)`, `AND_IN_B (1)`            | `XOR_OUT (1)`                               | `<rinku/logic/logic.h>`   |
| `Switch`      | `Rinku::Util`  |                                           | `SWITCH_OUT (1)`                            | `<rinku/util/switch.h>`   |
| `Bus`         | `Rinku::Util`  | `BUS_DATA_IN (64)`                        | `BUS_DATA_OUT (64)`                         | `<rinku/util/bus.h>`      |
| `Splitter<N>` | `Rinku::Util`  | `SPLITTER_IN (64)`                        | `SPLITTER_OUT_X (1)`</br>(`X` from 0 to 63) | `<rinku/util/splitter.h>` |
| `Joiner<N>`   | `Rinku::Util`  | `JOINER_IN_X (1)` <br/>(`X` from 0 to 63) | `JOINER_OUT (64)`                           | `<rinku/util/joiner.h>`   |

#### Switch Members
The `Switch` module does not have inputs that it acts upon. Instead, it can be operated using the functions `Switch::set(bool)` or `Switch::toggle()`.

#### `Splitter<N>` and `Joiner<N>`
Even though all splitters and joiners have 64 inputs/outputs available, the template parameter `N` should signify the number of inputs that are connected. When processing inputs and outputs, these classes will only update the first `N` inputs and outputs to optimize for speed.


## Building the System
### Step 1. Create a System Object

Once all modules have been defined, a system can be built by creating instances of these modules. First, we need to define a new `Rinku::System` object:

  ```cpp
  using namespace Rinku;
  
  System build() {
    System sys;
	// setup the system (see below)
  }
  ```
Alternatively, we can derive our own class from `System` and set it up in its constructor:

  ```cpp
  using namespace Rinku;
  
  class MySystem: public System {
  public:
    MySystem() {
	   // setup the system in the constructor
	}
  };
  ```
  
### Step 2. Add Module Instances to the System
Next, we use the `System::addModule` function-template to instantiate modules and add them to the system. This function returns a reference to the newly created module. For your convenience, it is recommended to use `auto&` to avoid typing the type more than once:

  ```cpp
  MySystem::MySystem() {
	auto& regA    = addModule<Register>();
	auto& regB    = addModule<Register>(42); // constructor argument
	auto& andGate = addModule<And>();
	
	// add more modules to the system, connect, init (see below)
  }
  ```
  
For completion's sake, macro's have been provided to add modules as well. These macro's do not add any additional diagnostics and should be used with care. In contrast to other macro's they do not expand to a complete statement; a parameter list that should be passed to the added module's constructor should be added explicitly.

| C++                          | Macro                          | Description                            |
|------------------------------|--------------------------------|----------------------------------------|
| `addModule<Module>(...)`     | `ADD_MODULE(Module)(...)`      | Add module to the system.              |
| `sys.addModule<Module>(...)` | `ADD_MODULE(sys, Module)(...)` | Add module explicitly to `sys` object. |

  ```cpp
  // From a member in a class deriving from System
  auto& regA = ADD_MODULE(Register)();   // no arguments to Register
  auto& regB = ADD_MODULE(Register)(42); // pass 42 to the Register constructor
  
  // Or when building a system object seperately
  System system;
  auto& regA = ADD_MODULE(system, Register)(42);
  ```


### Step 3. Make Connections
The system topology is defined by connections between inputs and outputs of different modules (you can even wire modules to themselves if you want). To make a new connection, regular C++ or a macro may be used. This macro also serves the purpose of passing debug-information to the connect-functions in order to produce easier to interpret error-messages or warnings when faulty connections are made.

| C++                                    | Macro                                       | Description                                                                  |
|----------------------------------------|---------------------------------------------|------------------------------------------------------------------------------|
| `mod.connect<Input, Output>(otherMod)` | `CONNECT_MOD(mod, Input, otherMod, Output)` | Connect `mod`'s input-signal `Input` to `otherMod`'s output-signal `Output`. |
| `mod.connect<Input, 1>()`              | `CONNECT_CONST(mod, Input, 1)`              | Connect `mod`'s input-signal `Input` to a constant signal with value `1`.    |

  ```cpp
  // C++
  andGate.connect<AND_IN_A, 1>();
  andGate.connect<AND_IN_B, 1>();
  regA.connect<REG_DATA_IN, REG_DATA_OUT>(regB);
  regB.connect<REG_EN, AND_OUT>(andGate);

  // Or when using macro's
  CONNECT_CONST(andGate, AND_IN_A, 1);
  CONNECT_CONST(andGate, AND_IN_B, 1);
  CONNECT_MOD(regA, REG_DATA_IN, regB, REG_DATA_OUT);
  CONNECT_MOD(regB, REG_EN, andGate, AND_OUT);
  ```
#### Inverted Outputs
Sometimes it makes sense to connect inputs to inverted outputs. You could design a module to act as an inverter but for convenience the `Rinku::Not<>` template, or `RINKU_NOT()` macro have been provided. When connecting a module to a negated output, simply wrap the output-signal in either the template or macro, depending on preference.

  ```cpp
  regA.connect<REG_EN, Not<AND_OUT>>(andGate);
  CONNECT(regA, REG_EN, andGate, NOT(AND_OUT));
  ```

#### Multiple Outputs to a Single Input
Rinku allows connecting multiple outputs to a single input. This is necessary for instance when creating a module that acts like a bus. When an input connects to multiple outputs, its resulting value is the bitwise OR of each of the outputs of the connected to it.

### 4. Special System Connections
The `System` is itself a module and has 4 input-signals (no outputs): `SYS_HLT`, `SYS_ERR`, `SYS_EXIT` and `SYS_EXIT_CODE`. These input signals can be connected to output signals in order to make the system halt, exit with an error or exit gracefully when issued. The system's `run()` function will return the value present at the `SYS_EXIT_CODE` input (8 bits). Special connect-functions are available in the System interface to make these connections, though they can also be made like any other connection (see above).

| C++                                   | Macro                                   | Description                                                   |
|---------------------------------------|-----------------------------------------|---------------------------------------------------------------|
| `connectHalt<Signal>(module)`         | `SYSTEM_HALT(Signal, module)`           | Connect the halt-signal                                       |
| `sys.connectHalt<Signal>(module)`     | `SYSTEM_HALT(sys, Signal, module)`      | Connect the halt-signal to a standalone `System` object.      |
| `connectError<Signal>(module)`        | `SYSTEM_ERROR(Signal, module)`          | Connect the error-signal                                      |
| `sys.connectError<Signal>(module)`    | `SYSTEM_ERROR(sys, Signal, module)`     | Connect the error-signal to a standalone `System` object.     |
| `connectExit<Signal>(module)`         | `SYSTEM_EXIT(Signal, module)`           | Connect the exit-signal                                       |
| `sys.connectExit<Signal>(module)`     | `SYSTEM_EXIT(sys, Signal, module)`      | Connect the exit-signal to a standalone `System` object.      |
| `connectExitCode<Signal>(module)`     | `SYSTEM_EXIT_CODE(Signal, module)`      | Connect the exit-code-signal (8-bit)                          |
| `sys.connectExitCode<Signal>(module)` | `SYSTEM_EXIT_CODE(sys, Signal, module)` | Connect the exit-code-signal to a standalone `System` object. |


In the example below, it is assumed that these connections are made inside a member (e.g. the constructor) of a class derived from `System`. If using a standalone `System` object, these functions can be called on that object instead.

  ```cpp
  // Using dedicated System members 
  connectHalt<AND_OUT>(andGate);  // halt when the And-gate goes high
  connectError<AND_OUT>(andGate); // error when the And-gate goes high
  connectExit<AND_OUT>(andGate);  // exit when the And-gate goes high
  connectExitCode<REG_OUT>(regA); // return value from regA when done
  
  // Using normal connect syntax
  connect<SYS_HLT, AND_OUT>(andGate); 
  // similarly for SYS_ERR, SYS_EXIT and SYS_EXIT_CODE
  
  // Using macro's
  SYSTEM_HALT(AND_OUT, andGate);
  SYSTEM_ERROR(AND_OUT, andGate);
  SYSTEM_EXIT(AND_OUT, andGate);
  SYSTEM_EXIT_CODE(REG_OUT, regA);

  // Using macro's on a System object
  System system;
  //...
  SYSTEM_HALT(system, AND_OUT, andGate);
  SYSTEM_ERROR(system, AND_OUT, andGate);
  SYSTEM_EXIT(system, AND_OUT, andGate);
  SYSTEM_EXIT_CODE(system, REG_OUT, regA);
  
  ```

## Running

When the system is fully setup, it can be initialized and run using the `System::init()` and `System::run()` functions. For example:

  ```cpp
  
  class MySystem: public System {
  public:
    MySystem() {
	   // Build the system by adding and connecting modules
	   init(); // initialize -> will lock the topology and reset all modules
	}
  };
  
  int main() {
	MySystem sys;
	int ret = sys.run();
	std::cout << "System finished with exit code " << ret << '\n';
  }
```
### Other `System` Functions
The table below shows the full interface for the `System` module.

| Method                                       | Description                                                                                |
|----------------------------------------------|--------------------------------------------------------------------------------------------|
| `ModuleType& addModule<ModuleType>(args...)` | Add a module to the system.                                                                |
| `void connectHalt<OutputSignal>(module)`     | Connect the halt signal.                                                                   |
| `void connectError<OutputSignal>(module)`    | Connect the error signal.                                                                  |
| `void connectExit<OutputSignal>(module)`     | Connect the exit signal.                                                                   |
| `void connectExitCode<OutputSignal>(module)` | Connect the exit-code signal.                                                              |
| `void init()`                                | Initialize the system.                                                                     |
| `signal_t run(resumeOnHalt = false)`         | Run the system continuously. If `resumeOnHalt` is set to true, the halt-signal is ignored. |
| `void step(resumeOnHalt = false)`            | Process one full clock-cycle (rising -> falling).                                          |

