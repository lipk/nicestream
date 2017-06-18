# nicestream

C++ input streams are notoriously clunky and inconvenient to use. **nicestream** tries to improve that by providing classes that
you can simply >> into the usual way to skip over unimportant fields or formatting data in the stream. Work in progress.

## Features

No more "trash" variables - use nice::skip to ignore the field you don't need:

    std::cin >> i >> nice::skip<int>() >> j;

Or even multiple fields:

    std::cin >> i >> nice::skip<int, double>() >> j;

Specify field separators with regular expressions:

    std::cin >> i >> nice::sep(" *, *") >> j;

## Limitations

The regex engine currently recognizes a limited subset of PCRE only.

**nicestream** is in a very early stage of development, so expect bugs :)
