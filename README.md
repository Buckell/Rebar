# Rebar
Fast, embeddable, JIT compiled language.

### Example Code
This is concept code. There is still some discussion on what the final language will exactly look like, especially with more specific things like the case for standard function names, but this is a close approximation to how the final language might look.

```
local StrUtil = Include("StringUtility");

local sb = new StrUtil::StringBuilder;

// Either .Append() or the += operator works!
sb.Append("Hello,");
sb += " ";
sb.Append("world!");

PrintLn(sb.ToString()); // Prints "Hello, world!"
```

### API Example

##### Simple Example
```cpp
#include "rebar/rebar.hpp"

int main() {
    // Create the environment. This is where the magic happens.
    rebar::environment env;

    // Compile a string to print "Hello, world!" three times
    // and return "Goodbye, world!"
    rebar::function main_func = env.compile_string(R"(
        for (local i = 0; i < 3; ++i) {
            PrintLn("Hello, world!");
        }

        return "Goodbye, world!";
    )");

    // Call our compiled function and get the return value.
    rebar::object return_object = main_func();

    std::cout << return_object << std::endl; // "Goodbye, world!"
}
```