# Rebar
Fast, embeddable, JIT compiled language.

### Example Code
This is concept code. There is still some discussion on what the final language will exactly look like, especially with more specific things like the case for standard function names, but this is a close approximation to how the final language might look.

```
// Global variables are defined with an identifier, an equal sign, 
// and the value (with the terminating semicolon of course).
Variable1 = "This variable is global!";

// Local variables are defined similarly to global variables but have
// a simple "local" prefix.
local Variable2 = "This variable is local!";

// Variable identifiers can be defined with alphanumeric characters and underscores.

// Standard "print" function. Outputs "Hello, world!" and a newline suffix to the console.
PrintLn("Hello, world!");

// Function declaration/definition.
// Similar to Javascript's function declaration.
function ExampleFunction() {
	PrintLn("This is a simple function!");
}
```

### Global Variables
Global variables are those that can always be accessed from any part of the code and from totally different files--as long as they are compiled and ran under the same environment.

##### Global Variables Demonstration
```cpp
rebar::environment env;

rebar::function first_file = env.compile_string("GlobalVar = \"Hello, world!\"; local LocalVar = \"Goodbye, world!\";");
first_file();

rebar::function second_file = env.compile_string("PrintLn(GlobalVar); PrintLn(LocalVar);");
second_file();
// Prints:
// Hello, world!
// null
```
Local variables are only valid for the lifetime of their scope, while global variables are valid for the lifetime of the environment.
