# User manual

## Installation

nicestream currently doesn't provide installer scripts, let alone binaries.
Since it consists of a meagerly two files (nicestream.(c|h)pp), it's probably
the best to just copy both of them into your own source tree, or pull it in via
a git submodule. Building nicestream doesn't require anything beyond the good
ol' STL and a C++11 capable compiler.

### Running unit tests

Pull the catch submodule and type 'make test' in a terminal. This builds and
also launches all the unit tests. Catch compiles quite slowly, be patient.

## Usage

Everything nicestream lives in the nstr namespace in nicestream.hpp.

### nstr::skip

skip serves to replace dummy variables that are used only to read ignored data
into. You could just insert a skip object parameterized with the appropriate
types to achieve the same effect:

    std::cin >> x >> skip<int, double, string>() >> y;

This is the same as writing

    int dummy1;
    double dummy2;
    string dummy3;
    std::cin >> x >> dummy1 >> dummy2 >> dummy3 >> y;

Only much neater.

### nstr::sep

sep specifies a separator. Some prefix of the stream must match the separator,
or you'll get an invalid_input exception:

    std::stringstream("10;20") >> i >> sep(";") >> j; // OK
    std::stringstream("10,20") >> i >> sep(";") >> j; // invalid_input

As usual with regular expressions, sep will match the longest possible sequence,
so

    std::stringstream(",,,,,,,abc") >> sep(",*") >> str;

...will yield "abc".

Supported regex features:

    * ., ?, +, *
    * Character classes defined with [...] and [^...]
    * Brace quantizers: {n,m}, {n,}, {n}
    * Union: (x|y)
    * Predefined classes: \d for digits, \s for whitespace, \w for alphanumeric
      plus _, and the complementers of those as \D, \W, and \S respectively.

Malformed regular expressions will yield an invalid_regex exception.

## Under the hood

nicestream constructs a nondeterministic finite automata for each regex-based
class, so it definitely has some overhead. (Although if speed is a concern, you
should probably avoid std::istream anyway).
