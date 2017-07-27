#ifndef NICEOUT_HPP_INCLUDED
#define NICEOUT_HPP_INCLUDED

#include <iostream>
#include <string>

namespace nstr {

template<typename ItorT>
class join_t {
    std::string sep;
    ItorT begin, end;

    template<typename T>
    friend std::ostream &operator <<(std::ostream &os, join_t<T> obj);

public:
    join_t(std::string &&sep, ItorT begin, ItorT end)
        : sep(std::move(sep)), begin(begin), end(end) {}
};

template<typename T>
std::ostream &operator <<(std::ostream &os, join_t<T> obj) {
    if (obj.begin != obj.end) {
        os << *obj.begin;
        while (++obj.begin != obj.end) {
            os << obj.sep << *obj.begin;
        }
    }
    return os;
}

template<typename ItorT>
join_t<ItorT> join(std::string &&sep, ItorT begin, ItorT end) {
    return join_t<ItorT>(std::move(sep), std::move(begin), std::move(end));
}

template<typename ContT>
join_t<typename ContT::const_iterator> join(std::string &&sep, const ContT& obj) {
    return join_t<typename ContT::const_iterator>(std::move(sep), obj.begin(), obj.end());
}

}

#endif

