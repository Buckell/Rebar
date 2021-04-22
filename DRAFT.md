```
// --- VARIABLE DEFINITIONS ---

// Local variables are available for the current scope and nested scopes.
local variable1 = "This is a local variable.";

// Global variables are available for the entire environment. All scopes and files
// ran under the same environment will be able to access the variable.
global variable2 = "This is a global variable.";

local gen = new Random.Generator;

local randint = gen.RandInt();

```