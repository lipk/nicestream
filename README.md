# nicestream

[![Build Status](https://travis-ci.org/lipk/nicestream.svg?branch=master)](https://travis-ci.org/lipk/nicestream)
[![GitHub tag](https://img.shields.io/github/tag/lipk/nicestream.svg)]()
[![License](https://img.shields.io/github/license/mashape/apistatus.svg)]()

C++ input streams are notoriously clunky and inconvenient to use. **nicestream** tries to improve that by providing classes that
you can simply >> into the usual way to skip over unimportant fields or formatting data in the stream. Work in progress.

## Features

No more "trash" variables - use nstr::skip to ignore the field you don't need:

    std::cin >> i >> nstr::skip<int>() >> j;

Or even multiple fields:

    std::cin >> i >> nstr::skip<int, double>() >> j;

Specify field separators with regular expressions:

    std::cin >> i >> nstr::sep(" *, *") >> j;

nstr::until instead of getline:

    std::cin >> nstr::until("[^\\]>", str);

Read sequences easily, into any container you fancy:

    std::cin >> nstr::split(";", "\n", some_container);

For details, see the [manual](MANUAL.md).

## Limitations

* The regex engine lacks some of the more fancy features.
* Doesn't support any kind of multibyte encoding.
