module;
#include <vector>
export module std:vector;
export namespace std {
    using std::vector;
    
    using std::erase;
    using std::erase_if;
    using std::operator!=;
    using std::operator==;
    using std::operator-;
}

#if defined(__GLIBCXX__) || defined(__GLIBCPP__)
export namespace __gnu_cxx {
    using __gnu_cxx::operator==;
    using __gnu_cxx::operator-;
}
#endif

// FIXME: We should put this in <new>.
export {
    using ::operator new;
}
