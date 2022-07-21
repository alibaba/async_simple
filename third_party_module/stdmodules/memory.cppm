module;
#include <memory>
export module std:memory;
export namespace std {
    using std::unique_ptr;
    using std::make_unique;
    using std::shared_ptr;
    using std::make_shared;
    using std::addressof;
    using std::allocator;
    using std::allocator_traits;

    using std::operator!=;
    using std::operator==;
}
