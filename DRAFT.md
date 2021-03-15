```
function RepeatString(to_repeat : string notnull, count : integer notnull = 1, truncate : integer/null) {
    if (truncate == null) {
        throw NoTruncateException "A truncate value must be specified.";
    }

    return (string * count)[:truncate];
}

local function Add(value, to_add = 1) {
    return value + to_add;
}

local test_string = "Hello, world!";

for (local i = 0; i < #test_string; i++) {
    PrintLn(test_string[i]);
}

```