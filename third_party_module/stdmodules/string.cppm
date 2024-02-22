module;
#include <string>
# 3 __FILE__ 1 3 // Enter "faked" system files since std is reserved module name
export module std:string;
export namespace std {
    using std::string;
    using std::basic_string;
    using std::string_view;
    using std::basic_string_view;

    using std::getline;
    
    using std::operator+;
    using std::operator==;
    using std::operator<;
    using std::operator>;
#if defined(__GLIBCXX__) && __GLIBCXX__ < 20200723
    using std::operator<=>;
#endif

    using std::to_string;

    inline namespace literals {
        using std::literals::operator""s;
    }
}

#if defined(__GLIBCXX__)
// libstd++ may inhibit implicit instantiations for the following
// specializations. See <allocator.h>.
// And the explicit instantiation should happen in allocator-inst.cc.
// However, I am not sure why, I can't see the symbols in libstdc++.so.
//
// To workaround the issues, I tried to instantiate the templates by myself
// here.
//
// Maybe this may produce some link errors in some other configurations.
extern "C++" {
    template class std::allocator<char>;
    template class std::allocator<wchar_t>;
}
#endif
