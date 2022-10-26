module;
#include <unordered_map>
export module std:unordered_map;
export namespace std {
    using std::unordered_map;
}

#if defined(__GLIBCXX__)
export namespace std {
    namespace __detail {
       using std::__detail::_Node_const_iterator;
#if __GLIBCXX__ < 20220728
       using std::__detail::operator==;
#endif
    }
}
#endif
