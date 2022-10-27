module;
#include <map>
export module std:map;
export namespace std {
    using std::map;
#if defined(__GLIBCXX_) && __GLIBCXX_ < 20200723
    using std::erase_if;
#endif
}
