# nicestream

[![Build Status](https://travis-ci.org/lipk/nicestream.svg?branch=master)](https://travis-ci.org/lipk/nicestream)
[![Issues in Ready](https://badge.waffle.io/lipk/nicestream.svg?label=ready&title=Ready)](http://waffle.io/lipk/nicestream)
[![Issues In Progress](https://badge.waffle.io/lipk/nicestream.svg?label=In%20Progress&title=In%20Progress)](http://waffle.io/lipk/nicestream)

C++ input streams are notoriously clunky and inconvenient to use. **nicestream** tries to improve that by providing classes that
you can simply >> into the usual way to skip over unimportant fields or formatting data in the stream. Work in progress.

## Features

No more "trash" variables - use nice::skip to ignore the field you don't need:

    std::cin >> i >> nstr::skip<int>() >> j;

Or even multiple fields:

    std::cin >> i >> nstr::skip<int, double>() >> j;

Specify field separators with regular expressions:

    std::cin >> i >> nstr::sep(" *, *") >> j;

For details, see the [manual](manual.md).

## Limitations

The regex engine currently recognizes a limited subset of PCRE only.

**nicestream** is in a very early stage of development, so expect bugs :)
