# User manual

## Installation

nicestream currently doesn't provide installer scripts, let alone binaries.
However, it consists of only four files (nicestream and nfa cpp and hpp), so it
shouldn't be too much hassle to just include the whole thing in your source
tree. The only thing you need to build nicestream is a C++11 capable compiler.

### Running unit tests

Pull the catch submodule and type 'cmake . && make' in a terminal to build the
unit tests. You can then run them with './nice_tests'. 

## Usage

Everything nicestream lives in the nstr namespace in nicestream.hpp.

### Supported regex features

nicestream uses its own regex engine, which supports the following features:

* ., ?, +, *
* Character classes defined with [...] and [^...]
* Brace quantizers: {n,m}, {n,}, {n}
* Union: (x|y)
* Predefined classes: \d for digits, \s for whitespace, \w for alphanumeric
  plus _, and the complementers of those as \D, \W, and \S respectively.

Malformed regular expressions will yield an invalid_regex exception.

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

### nstr::until

until takes two parameters: a regex and a string reference. It stops reading
from the stream when it finds a matching subsequence and dumps the data, not
including the terminating sequence, into the string. Like this:

    std::stringstream("yadda yadda. yadda" >> nstr::until("\\.", str);

str now contains the string "yadda yadda".

### nstr::all

Simply reads all data from the stream and puts it into a string. Example:

    std::stringstream("this text will all be read into str") >> nstr::all(str);

### nstr::split

split can be used to read a series of values, separated and terminated by some regular expressions,
into a container. A simple example with ints:

    std::vector<int> vec;
    std::stringstream("1,2,3,4\n") >> nstr::split(",", "\n", vec);

It does not clear the target so any values already in the container will remain unchanged. Adding
elements is done via an std::inserter at end(), so for sequential containers, items will be
appended.

Conversion from string to the target type is done via a stringstream, except for strings, which are
just passed along verbatim:

    std::stringstream("a a,b b;") >> nstr::split(",", ";", strvec);

...will read two items, "a a" and "b b".

## Under the hood

nicestream constructs a nondeterministic finite automata for each regex-based
class, so it definitely has some overhead. (Although if speed is a concern, you
should probably avoid std::istream anyway).
