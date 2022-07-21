module;
#include <exception>
export module std:exception;
export namespace std {
    using std::exception_ptr;
    using std::exception;
}

#if defined(__GLIBCXX__) || defined(__GLIBCPP__)
export namespace std::__exception_ptr {
    using std::__exception_ptr::operator==;
}
#endif
