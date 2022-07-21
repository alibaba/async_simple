module;
#include <unordered_map>
export module std:unordered_map;
export namespace std {
    using std::unordered_map;
}

#if defined(__GLIBCXX__) || defined(__GLIBCPP__)
export namespace std {
    namespace __detail {
       using std::__detail::_Node_const_iterator;
       using std::__detail::operator==;
    }
}
#endif
